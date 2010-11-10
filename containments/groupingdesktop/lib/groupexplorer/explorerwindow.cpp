/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *   Copyright 2010 Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "explorerwindow.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <QPainter>
#include <QTimer>
#include <QGraphicsLayout>

#include <kwindowsystem.h>
#include <netwm.h>
#include <KIconLoader>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>

#include "groupexplorer.h"

ExplorerWindow *ExplorerWindow::s_instance = 0;

ExplorerWindow::ExplorerWindow(QWidget *parent)
   : QWidget(parent),
     m_location(Plasma::Floating),
     m_layout(new QBoxLayout(QBoxLayout::TopToBottom, this)),
     m_background(new Plasma::FrameSvg(this)),
     m_corona(0),
     m_view(0),
     m_groupManager(0),
     m_graphicsWidget(0)
{
    Q_UNUSED(parent)

    m_background->setImagePath("dialogs/background");
    m_background->setContainsMultipleImages(true);

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky | NET::KeepAbove);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocus(Qt::ActiveWindowFocusReason);
    setLocation(Plasma::BottomEdge);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), Qt::transparent);
    setPalette(pal);

    Plasma::WindowEffects::overrideShadow(winId(), true);

    m_layout->setContentsMargins(0, 0, 0, 0);

    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(onActiveWindowChanged(WId)));
    connect(m_background, SIGNAL(repaintNeeded()), SLOT(backgroundChanged()));
    m_adjustViewTimer = new QTimer(this);
    m_adjustViewTimer->setSingleShot(true);
    connect(m_adjustViewTimer, SIGNAL(timeout()), this, SLOT(syncToGraphicsWidget()));
}

ExplorerWindow::~ExplorerWindow()
{
    if (m_groupManager) {
        if (m_corona) {
            m_corona->removeOffscreenWidget(m_groupManager);
        }
    }

    delete m_groupManager;
    delete m_view;

    s_instance = 0;
}

ExplorerWindow *ExplorerWindow::instance()
{
    if (!s_instance) {
        s_instance = new ExplorerWindow();
    }

    return s_instance;
}

void ExplorerWindow::adjustSize(int screen)
{
    QSize screenSize = m_corona->screenGeometry(screen).size();

    setMaximumSize(screenSize);
}

void ExplorerWindow::backgroundChanged()
{
    Plasma::Location l = m_location;
    m_location = Plasma::Floating;
    setLocation(l);
    update();
}

void ExplorerWindow::setContainment(Plasma::Containment *containment)
{
    if (containment == m_containment.data()) {
        return;
    }
    m_containment = containment;

    if (m_containment) {
        disconnect(m_containment.data(), 0, this, 0);
    }

    if (!containment) {
        return;
    }
    m_corona = m_containment.data()->corona();

    foreach (Plasma::Containment *containment, m_corona->containments()) {
        connect(containment, SIGNAL(toolBoxToggled()), this, SLOT(close()));
    }

    if (m_groupManager) {
        m_groupManager->setContainment(containment);
    }
}

Plasma::Containment *ExplorerWindow::containment() const
{
    return m_containment.data();
}

void ExplorerWindow::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (m_graphicsWidget) {
        m_graphicsWidget->removeEventFilter(this);
    }

    m_graphicsWidget = widget;

    if (widget) {
        if (!layout()) {
            QVBoxLayout *lay = new QVBoxLayout(this);
            lay->setMargin(0);
            lay->setSpacing(0);
        }

        if (!m_view) {
            m_view = new QGraphicsView(this);
            m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_view->setFrameShape(QFrame::NoFrame);
            m_view->viewport()->setAutoFillBackground(false);
            layout()->addWidget(m_view);
        }

        m_view->setScene(widget->scene());

        //try to have the proper size -before- showing the dialog
        m_view->centerOn(widget);
        if (widget->layout()) {
            widget->layout()->activate();
        }
        static_cast<QGraphicsLayoutItem *>(widget)->updateGeometry();
        widget->resize(widget->size().expandedTo(widget->effectiveSizeHint(Qt::MinimumSize)));

        syncToGraphicsWidget();

        //adjustSizeTimer->start(150);

        widget->installEventFilter(this);
    } else {
        delete m_view;
        m_view = 0;
    }
}

void ExplorerWindow::syncToGraphicsWidget()
{
    m_adjustViewTimer->stop();
    if (m_view && m_graphicsWidget) {
        QSize prevSize = size();

        //set the sizehints correctly:
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);

        QRect screenRect;
        if (m_containment) {
            screenRect = m_corona->screenGeometry(m_containment.data()->screen());
        } else {
            return;
        }
        QSize maxSize = KWindowSystem::workArea().intersect(screenRect).size();

        QSize windowSize;
        if (m_location == Plasma::LeftEdge || m_location == Plasma::RightEdge) {
            windowSize = QSize(qMin(int(m_graphicsWidget->size().width()) + left + right, maxSize.width()), maxSize.height());
            m_graphicsWidget->resize(m_graphicsWidget->size().width(), windowSize.height());
        } else {
            windowSize = QSize(maxSize.width(), qMin(int(m_graphicsWidget->size().height()) + top + bottom, maxSize.height()));
            m_graphicsWidget->resize(windowSize.width(), m_graphicsWidget->size().height());
        }

        resize(windowSize);

        setMinimumSize(windowSize);

        updateGeometry();

        //reposition and resize the view.
        //force a valid rect, otherwise it will take up the whole scene
        QRectF sceneRect(m_graphicsWidget->sceneBoundingRect());

        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        m_view->setSceneRect(sceneRect);

        m_view->centerOn(m_graphicsWidget);

    }
}

bool ExplorerWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_graphicsWidget &&
        (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
        m_adjustViewTimer->start(150);
    }

    return QWidget::eventFilter(watched, event);
}

void ExplorerWindow::setLocation(const Plasma::Location &loc)
{
    if (m_location == loc) {
        return;
    }

    Plasma::WindowEffects::slideWindow(this, loc);

    m_location = loc;

    switch (loc) {
    case Plasma::LeftEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::RightBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        setContentsMargins(0, 0, m_background->marginSize(Plasma::RightMargin), 0);
        break;

    case Plasma::RightEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        setContentsMargins(m_background->marginSize(Plasma::LeftMargin), 0, 0, 0);
        break;

    case Plasma::TopEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::BottomBorder);
        m_layout->setDirection(QBoxLayout::BottomToTop);
        setContentsMargins(0, 0, 0, m_background->marginSize(Plasma::BottomMargin));
        break;

    case Plasma::BottomEdge:
    default:
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder);
        m_layout->setDirection(QBoxLayout::TopToBottom);
        setContentsMargins(0, m_background->marginSize(Plasma::TopMargin), 0, 0);
        break;
    }

    if (m_groupManager) {
        m_groupManager->setLocation(location());
    }

    resize(sizeHint());
}

QPoint ExplorerWindow::positionForPanelGeometry(const QRect &panelGeom) const
{
    int screen;
    if (m_containment) {
        screen = m_containment.data()->screen();
    } else {
        return QPoint();
    }

    QRect screenGeom = m_corona->screenGeometry(screen);

    switch (m_location) {
    case Plasma::LeftEdge:
        return QPoint(panelGeom.right(), screenGeom.top());
        break;
    case Plasma::RightEdge:
        return QPoint(panelGeom.left() - width(), screenGeom.top());
        break;
    case Plasma::TopEdge:
        return QPoint(screenGeom.left(), panelGeom.bottom());
        break;
    case Plasma::BottomEdge:
    default:
        return QPoint(screenGeom.left(), panelGeom.top() - height());
        break;
    }
}

Plasma::Location ExplorerWindow::location() const
{
    return m_location;
}

Qt::Orientation ExplorerWindow::orientation() const
{
    if (m_location == Plasma::LeftEdge || m_location == Plasma::RightEdge) {
        return Qt::Vertical;
    }

    return Qt::Horizontal;
}


void ExplorerWindow::showGroupExplorer()
{
    if (!m_groupManager) {
        m_groupManager = new GroupExplorer(location());

        m_corona->addOffscreenWidget(m_groupManager);
        m_groupManager->show();

        m_groupManager->setContainment(m_containment.data());
        m_groupManager->setLocation(location());
        if (orientation() == Qt::Horizontal) {
            m_groupManager->resize(width(), m_groupManager->size().height());
        } else {
            m_groupManager->resize(m_groupManager->size().width(), height());
        }

        m_groupManager->setIconSize(KIconLoader::SizeHuge);

        setGraphicsWidget(m_groupManager);

        connect(m_groupManager, SIGNAL(closeClicked()), this, SLOT(close()));
    } else {
        m_groupManager->setLocation(location());
        m_groupManager->show();
        setGraphicsWidget(m_groupManager);
    }

}

bool ExplorerWindow::isControllerViewVisible() const
{
    return m_view && m_view->isVisible();
}

Plasma::FrameSvg *ExplorerWindow::background() const
{
    return m_background;
}

void ExplorerWindow::onActiveWindowChanged(WId id)
{
    Q_UNUSED(id)

    //if the active window isn't the plasma desktop and the widgets explorer is visible,
    //then close the panel controller
    if (QApplication::activeWindow() == 0 || (QApplication::activeWindow()->winId() != KWindowSystem::activeWindow())) {
        if (m_view && m_view->isVisible() && !isActiveWindow()) {
            //close();
        }
    }
}

void ExplorerWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source );

    m_background->paintFrame(&painter);
}

void ExplorerWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}

void ExplorerWindow::resizeEvent(QResizeEvent * event)
{
    m_background->resizeFrame(size());

    Plasma::WindowEffects::enableBlurBehind(effectiveWinId(), true, m_background->mask());

    qDebug() << "ExplorerWindow::resizeEvent" << event->oldSize()<<event->size();

    QWidget::resizeEvent(event);
}

#include "explorerwindow.moc"
