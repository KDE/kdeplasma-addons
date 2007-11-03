/***************************************************************************
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kolourpicker.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qicon.h>
#include <qiconengine.h>
#include <qimage.h>
#include <qmimedata.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>

#include <plasma/widgets/boxlayout.h>
#include <plasma/widgets/pushbutton.h>

static KMenu* buildMenuForColor(const QColor &color)
{
    KMenu *menu = new KMenu();
    QAction *act = menu->addAction(KIcon("text"), QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    act->setData(color);
    QString htmlName = color.name();
    QString htmlNameUp = htmlName.toUpper();
    act = menu->addAction(KIcon("text-html"), htmlName);
    act->setData(color);
    act = menu->addAction(KIcon("text-html"), htmlName.mid(1));
    act->setData(color);
    if (htmlNameUp != htmlName)
    {
        act = menu->addAction(KIcon("text-html"), htmlNameUp);
        act->setData(color);
        act = menu->addAction(KIcon("text-html"), htmlNameUp.mid(1));
        act->setData(color);
    }
    return menu;
}


class ColorIconEngine : public QIconEngine
{
    public:
        ColorIconEngine(const QColor &color);
        virtual ~ColorIconEngine();

        virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state);
        virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);

    public:
        QColor m_color;
};

ColorIconEngine::ColorIconEngine(const QColor &color)
    : m_color(color)
{
}

ColorIconEngine::~ColorIconEngine()
{
}

void ColorIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_color);
    painter->drawEllipse(rect);
}

QPixmap ColorIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pix(size);
    pix.fill(QColor(0,0,0,0));
    QPainter p(&pix);
    paint(&p, pix.rect(), mode, state);
    p.end();
    return pix;
}


Kolourpicker::Kolourpicker(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_grabWidget(0)
{
    setDrawStandardBackground(false);

    Plasma::BoxLayout *mainlay = new Plasma::BoxLayout(Plasma::BoxLayout::TopToBottom, 0);
    setLayout(mainlay);
    mainlay->setMargin(0);
    mainlay->setSpacing(4);

    m_grabButton = new Plasma::PushButton(this);
    mainlay->addItem(m_grabButton);
    m_grabButton->setIcon(KIcon("color-picker"));
    m_grabButton->setIconSize(QSizeF(22, 22));
    connect(m_grabButton, SIGNAL(clicked()), this, SLOT(grabClicked()));

    m_historyButton = new Plasma::PushButton(this);
    mainlay->addItem(m_historyButton);
    m_historyButton->setEnabled(false);
    m_historyButton->setIcon(KIcon("color-picker"));
    m_historyButton->setIcon(KIcon(QIcon(new ColorIconEngine(Qt::gray))));
    m_historyButton->setIconSize(QSizeF(22, 22));
    connect(m_historyButton, SIGNAL(clicked()), this, SLOT(historyClicked()));

    KMenu *menu = new KMenu();
    menu->addTitle(i18n("History"));
    m_historyMenu = menu;
    m_historyMenu->addSeparator();
    QAction *act = m_historyMenu->addAction(KIcon("edit-clear-history"), i18n("Clear History"));
    connect(act, SIGNAL(triggered(bool)), this, SLOT(clearHistory()));

    resize(QSizeF(22, 50));
}

Kolourpicker::~Kolourpicker()
{
    clearHistory();
    delete m_historyMenu;
}

bool Kolourpicker::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == m_grabButton && event->type() == QEvent::GraphicsSceneMouseRelease)
    {
        m_grabWidget = static_cast<QGraphicsSceneMouseEvent *>(event)->widget();
        if (m_grabWidget && m_grabWidget->parentWidget())
        {
            m_grabWidget = m_grabWidget->parentWidget();
        }
        if (m_grabWidget)
        {
            m_grabWidget->installEventFilter(this);
        }
    }
    return false;
}

bool Kolourpicker::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_grabWidget && event->type() == QEvent::MouseButtonRelease)
    {
        m_grabWidget->removeEventFilter(this);
        m_grabWidget->releaseMouse();

        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        QPixmap pix = QPixmap::grabWindow(m_grabWidget->winId(), me->globalPos().x(), me->globalPos().y(), 1, 1);
        QImage img = pix.toImage();
        QColor color(img.pixel(0, 0));

        kDebug() << event->type() << me->globalPos() << color;

        addColor(color);

        KMenu *newmenu = buildMenuForColor(color);
        newmenu->addTitle(QIcon(new ColorIconEngine(color)), i18n("Copy Color Value"), newmenu->actions().first());
        connect(newmenu, SIGNAL(triggered(QAction*)), this, SLOT(colorActionTriggered(QAction*)));
        newmenu->exec(QCursor::pos());
        delete newmenu;
    }
    return Plasma::Applet::eventFilter(watched, event);
}

QVariant Kolourpicker::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneChange)
    {
        QMetaObject::invokeMethod(this, "installFilter", Qt::QueuedConnection);
    }
    return Plasma::Applet::itemChange(change, value);
}

void Kolourpicker::grabClicked()
{
    if (m_grabWidget)
    {
        m_grabWidget->grabMouse();
    }
}

void Kolourpicker::historyClicked()
{
    m_historyMenu->popup(QCursor::pos());
}

void Kolourpicker::colorActionTriggered(QAction *act)
{
    if (!act)
        return;

    QColor color = qvariant_cast<QColor>(act->data());
    if (!color.isValid())
        return;

    QMimeData *mime = new QMimeData();
    mime->setColorData(color);
    mime->setText(act->text());
    QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

void Kolourpicker::clearHistory()
{
    m_historyButton->setEnabled(false);
    m_historyButton->setIcon(KIcon(QIcon(new ColorIconEngine(Qt::gray))));
    QHash<QColor, QAction *>::ConstIterator it = m_menus.begin(), itEnd = m_menus.end();
    for ( ; it != itEnd; ++it )
    {
        m_historyMenu->removeAction(*it);
        delete *it;
    }
    m_menus.clear();
}

void Kolourpicker::installFilter()
{
    m_grabButton->installSceneEventFilter(this);
}

void Kolourpicker::addColor(const QColor &color)
{
    QHash<QColor, QAction *>::ConstIterator it = m_menus.find(color);
    if (it != m_menus.end())
        return;

    KMenu *newmenu = buildMenuForColor(color);
    QAction *act = newmenu->menuAction();
    act->setIcon(QIcon(new ColorIconEngine(color)));
    act->setText(QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    connect(newmenu, SIGNAL(triggered(QAction*)), this, SLOT(colorActionTriggered(QAction*)));
    m_historyMenu->insertMenu(m_historyMenu->actions().at(1), newmenu);
    m_historyButton->setIcon(KIcon(QIcon(new ColorIconEngine(color))));
    m_menus.insert(color, act);
    m_historyButton->setEnabled(true);
}

#include "kolourpicker.moc"
