/***************************************************************************
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
 
#include "magnifique.h"
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneWheelEvent>
#include <QApplication>
#include <QLayout>
#include <QSlider>
#include <QVBoxLayout>

#include <KWindowSystem>
#include <KIcon>

#include <Plasma/IconWidget>
#include <Plasma/Theme>
#include <Plasma/Dialog>
#include <Plasma/ToolTipManager>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/View>

Magnifique::Magnifique(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_view(0),
      m_mainWindow(0)
{
    resize(48, 48);
}


Magnifique::~Magnifique()
{
    delete m_mainWindow;
}

void Magnifique::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    Plasma::IconWidget *icon = new Plasma::IconWidget(this);
    icon->setIcon("zoom-in");
    layout->addItem(icon);
    setAspectRatioMode(Plasma::ConstrainedSquare);

    connect(icon, SIGNAL(clicked()), this, SLOT(toggleView()));

    Plasma::ToolTipContent data;
    data.setMainText(i18n("Magnifying glass"));
    data.setSubText(i18n("See the contents of your desktop through the windows"));
    data.setImage(KIcon("zoom-in"));
    Plasma::ToolTipManager::self()->setContent(this, data);
}

void Magnifique::toggleView()
{
    if (!m_mainWindow) {
        m_mainWindow = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(m_mainWindow);
        layout->setContentsMargins(0,0,0,0);
        m_view = new QGraphicsView(m_mainWindow);
        m_view->setScene(scene());

        m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setWindowTitle(i18n("Plasma Magnifier"));

        m_slider = new QSlider(Qt::Horizontal, m_mainWindow);
        m_slider->setMinimum(-2);
        m_slider->setMaximum(2);
        m_slider->setPageStep(1);
        connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)));
        layout->addWidget(m_view);
        layout->addWidget(m_slider);
    }

    KConfigGroup conf = config();

    if (m_mainWindow->isVisible()) {
        conf.writeEntry("Geometry", m_mainWindow->geometry());
        conf.writeEntry("Zoom", m_view->transform().m11());
        m_mainWindow->removeEventFilter(this);
        m_mainWindow->deleteLater();
        m_mainWindow = 0;
    } else {
        QRect geom = conf.readEntry("Geometry", QRect(0, 0, 200, 200));
        int zoom = conf.readEntry("Zoom", 1);

        m_mainWindow->setGeometry(geom);

        QTransform viewTransform;
        viewTransform.setMatrix(zoom, 0, 0,  0, zoom, 0,  0, 0, 1);

        m_view->setTransform(viewTransform);

        m_mainWindow->show();
        syncViewToScene();
        m_mainWindow->installEventFilter(this);
    }
}

void Magnifique::setZoom(int zoom)
{
    QTransform transform;
    if (zoom > 0) {
        transform.scale(zoom*2, zoom*2);
    } else if (zoom < 0) {
        transform.scale(1/(-(qreal)zoom*2), 1/(-(qreal)zoom*2));
    }
    m_view->setTransform(transform);
    syncViewToScene();
}

void Magnifique::syncViewToScene()
{
    QRect mappedRect(m_view->transform().inverted().mapRect(QRect(QPoint(0,0), m_view->size())));

    QPoint viewPos = m_view->mapToGlobal(m_view->pos());
    QRect originalRect(scenePosFromScreenPos(QPoint(qMax(viewPos.x(), 0), qMax(viewPos.y(), 0))).toPoint(), m_view->size());

    mappedRect.moveCenter(originalRect.center());

    //avoid to show negative coordinates in the view - we don't want to show the panel :)
    mappedRect.moveTop(qMax(mappedRect.top(), 0));
    mappedRect.moveLeft(qMax(mappedRect.left(), 0));

    m_view->setSceneRect(mappedRect);
}

bool Magnifique::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Move || event->type() == QEvent::Resize) {
        syncViewToScene();
        return true;
    }
    return Applet::eventFilter(watched, event);
}

void Magnifique::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    qreal delta = 1;

    //don't allow too big zooming, for speed reason :/

    if (event->delta() < 0 && m_view->transform().m11() > 0.25) {
        delta = 0.5;
    } else if(event->delta() > 0 && m_view->transform().m11() < 4) {
        delta = 2;
    }

    m_view->scale(delta, delta);
    qreal factor = m_view->transform().m11();
    if (factor > 1) {
        m_slider->setValue(factor/2);
    } else if (factor < 1 && factor > 0) {
        m_slider->setValue(-((1/factor)/2));
    } else {
       m_slider->setValue(0);
    }
    syncViewToScene();
}

QPointF Magnifique::scenePosFromScreenPos(const QPoint &pos) const
{
    Plasma::Corona *corona = containment()->corona();
    Plasma::Containment *cont = 0;
    if (corona) {
        cont = corona->containmentForScreen(containment()->screen(), KWindowSystem::currentDesktop()-1);
        if (!cont) {
            cont = corona->containmentForScreen(containment()->screen(), -1);
        }
    }
    if (!corona || !cont) {
        return QPoint();
    }

    //get the stacking order of the toplevel windows and remove the toplevel view that's
    //only here while dragging, since we're not interested in finding that.
    QList<WId> order = KWindowSystem::stackingOrder();

    Plasma::View *found = 0;
    foreach (QWidget *w, QApplication::topLevelWidgets()) {
        Plasma::View *v = qobject_cast<Plasma::View *>(w);

        if (v && v->containment() == cont) {
            found = v;
            break;
        }
    }

    if (!found) {
        return QPointF();
    }

    return found->mapToScene(found->mapFromGlobal(pos));
}

#include "magnifique.moc"
