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
#include <qgraphicslinearlayout.h>
#include <qicon.h>
#include <qiconengine.h>
#include <qimage.h>
#include <qmimedata.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtoolbutton.h>

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>

#include <plasma/widgets/toolbutton.h>

#include <config-kolourpicker.h>

#if defined(KOLOURPICKER_X11_LIB)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>
#endif

static KMenu* buildMenuForColor(const QColor &color)
{
    KMenu *menu = new KMenu();
    const QVariant colorData = qVariantFromValue(color);
    QAction *act = menu->addAction(KIcon("draw-text"), QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    act->setData(colorData);
    QString htmlName = color.name();
    QString htmlNameUp = htmlName.toUpper();
    KIcon mimeIcon("text-html");
    act = menu->addAction(mimeIcon, htmlName);
    act->setData(colorData);
    act = menu->addAction(mimeIcon, htmlName.mid(1));
    act->setData(colorData);
    if (htmlNameUp != htmlName)
    {
        act = menu->addAction(mimeIcon, htmlNameUp);
        act->setData(colorData);
        act = menu->addAction(mimeIcon, htmlNameUp.mid(1));
        act->setData(colorData);
    }
    return menu;
}

static QColor pickColor(const QPoint &point)
{
#if defined(KOLOURPICKER_X11_LIB)
/*
  It seems the Qt4 stuff returns a null grabbed pixmap when the Display
  has ARGB visuals.
  Then, access directly to the screen pixels using the X API.
*/
    Window root = RootWindow(QX11Info::display(), QX11Info::appScreen());
    XImage *ximg = XGetImage(QX11Info::display(), root, point.x(), point.y(), 1, 1, -1, ZPixmap);
    unsigned long xpixel = XGetPixel(ximg, 0, 0);
    XDestroyImage(ximg);
    XColor xcol;
    xcol.pixel = xpixel;
    xcol.flags = DoRed | DoGreen | DoBlue;
    XQueryColor(QX11Info::display(), DefaultColormap(QX11Info::display(), QX11Info::appScreen()), &xcol);
    return QColor::fromRgbF(xcol.red / 65535.0, xcol.green / 65535.0, xcol.blue / 65535.0);
#else
    QDesktopWidget *desktop = QApplication::desktop();
    QPixmap pix = QPixmap::grabWindow(desktop->winId(), point.x(), point.y(), 1, 1);
    QImage img = pix.toImage();
    return QColor(img.pixel(0, 0));
#endif
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
    p.setRenderHint(QPainter::Antialiasing, true);
    paint(&p, pix.rect(), mode, state);
    p.end();
    return pix;
}


class ColorIcon : public QIcon
{
    public:
        ColorIcon(const QColor &color);
};

ColorIcon::ColorIcon(const QColor &color)
    : QIcon(new ColorIconEngine(color))
{
}

Kolourpicker::Kolourpicker(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_grabWidget(0)
{
    resize(40, 80);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    QGraphicsLinearLayout *mainlay = new QGraphicsLinearLayout(Qt::Vertical);
    setLayout(mainlay);
    mainlay->setSpacing(4);
    mainlay->setContentsMargins(0.0, 0.0, 0.0, 0.0);

    m_grabButton = new Plasma::ToolButton(this);
    m_grabButton->setMinimumSize(20, 20);
    mainlay->addItem(m_grabButton);
    m_grabButton->nativeWidget()->setIcon(KIcon("color-picker"));
    m_grabButton->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(m_grabButton, SIGNAL(clicked()), this, SLOT(grabClicked()));

    m_historyButton = new Plasma::ToolButton(this);
    m_historyButton->setMinimumSize(20, 20);
    mainlay->addItem(m_historyButton);
    m_historyButton->setEnabled(false);
    m_historyButton->nativeWidget()->setIcon(ColorIcon(Qt::gray));
    m_historyButton->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(m_historyButton, SIGNAL(clicked()), this, SLOT(historyClicked()));

    KMenu *menu = new KMenu();
    menu->addTitle(i18n("History"));
    m_historyMenu = menu;
    m_historyMenu->addSeparator();
    QAction *act = m_historyMenu->addAction(KIcon("edit-clear-history"), i18n("Clear History"));
    connect(act, SIGNAL(triggered(bool)), this, SLOT(clearHistory()));
}

Kolourpicker::~Kolourpicker()
{
    clearHistory(false);
    delete m_historyMenu;
}

void Kolourpicker::init()
{
    KConfigGroup cg = config();

    QList<QString> colorList = cg.readEntry("Colors", QList<QString>());
    Q_FOREACH (const QString &color, colorList)
    {
        addColor(QColor(color), false);
    }
}

void Kolourpicker::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar) {
            setBackgroundHints(Plasma::Applet::StandardBackground);
        } else {
            setBackgroundHints(Plasma::Applet::NoBackground);
        }
    }

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::SizeConstraint) {
        QGraphicsLinearLayout *l = dynamic_cast<QGraphicsLinearLayout *>(layout());
        if (formFactor() == Plasma::Horizontal && size().height() < 40) {
            l->setOrientation(Qt::Horizontal);
        } else {
            l->setOrientation(Qt::Vertical);
        }
    }
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

        const QColor color = pickColor(me->globalPos());

        kDebug() << event->type() << me->globalPos() << color;

        addColor(color);

        KMenu *newmenu = buildMenuForColor(color);
        newmenu->addTitle(ColorIcon(color), i18n("Copy Color Value"), newmenu->actions().first());
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
        m_grabWidget->grabMouse(Qt::CrossCursor);
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
    mime->setText(act->text().remove(QChar('&')));
    QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

void Kolourpicker::clearHistory(bool save)
{
    m_historyButton->setEnabled(false);
    m_historyButton->nativeWidget()->setIcon(ColorIcon(Qt::gray));
    QHash<QColor, QAction *>::ConstIterator it = m_menus.constBegin(), itEnd = m_menus.constEnd();
    for ( ; it != itEnd; ++it )
    {
        m_historyMenu->removeAction(*it);
        delete *it;
    }
    m_menus.clear();
    m_colors.clear();

    if (save)
    {
        KConfigGroup cg = config();
        saveData(cg);
    }
}

void Kolourpicker::installFilter()
{
    m_grabButton->installSceneEventFilter(this);
}

void Kolourpicker::addColor(const QColor &color, bool save)
{
    QHash<QColor, QAction *>::ConstIterator it = m_menus.constFind(color);
    if (it != m_menus.constEnd())
        return;

    KMenu *newmenu = buildMenuForColor(color);
    QAction *act = newmenu->menuAction();
    ColorIcon colorIcon(color);
    act->setIcon(colorIcon);
    act->setText(QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    connect(newmenu, SIGNAL(triggered(QAction*)), this, SLOT(colorActionTriggered(QAction*)));
    m_historyMenu->insertMenu(m_historyMenu->actions().at(1), newmenu);
    m_historyButton->nativeWidget()->setIcon(colorIcon);
    m_menus.insert(color, act);
    m_colors.append(color.name());
    m_historyButton->setEnabled(true);
    if (save)
    {
        KConfigGroup cg = config();
        saveData(cg);
    }
}

void Kolourpicker::saveData(KConfigGroup &cg)
{
    cg.writeEntry("Colors", m_colors);

    emit configNeedsSaving();
}

#include "kolourpicker.moc"
