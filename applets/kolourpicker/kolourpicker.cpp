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

    if (htmlNameUp != htmlName) {
        act = menu->addAction(mimeIcon, htmlNameUp);
        act->setData(colorData);
        act = menu->addAction(mimeIcon, htmlNameUp.mid(1));
        act->setData(colorData);
    }

    menu->addSeparator();
    act = menu->addAction(mimeIcon, "Latex Color");
    act->setData(colorData);

    act = menu->addAction(mimeIcon, htmlName + QString::fromLatin1("ff"));
    act->setData(colorData);
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
    : Plasma::Applet(parent, args),
      m_grabWidget(0)
{
    resize(40, 80);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    QGraphicsLinearLayout *mainlay = new QGraphicsLinearLayout(Qt::Vertical);
    setLayout(mainlay);
    mainlay->setSpacing(4);
    mainlay->setContentsMargins(0.0, 0.0, 0.0, 0.0);

    m_grabWidget = new QWidget( 0,  Qt::X11BypassWindowManagerHint );
    m_grabWidget->move( -1000, -1000 );

    m_grabButton = new Plasma::ToolButton(this);
    m_grabButton->setMinimumSize(20, 20);
    mainlay->addItem(m_grabButton);
    m_grabButton->nativeWidget()->setIcon(KIcon("color-picker"));
    m_grabButton->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(m_grabButton, SIGNAL(clicked()), this, SLOT(grabClicked()));

    m_configAndHistory = new Plasma::ToolButton(this);
    m_configAndHistory->setMinimumSize(20, 20);
    mainlay->addItem(m_configAndHistory);

    m_configAndHistory->nativeWidget()->setIcon(ColorIcon(Qt::gray));
    m_configAndHistory->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(m_configAndHistory, SIGNAL(clicked()), this, SLOT(historyClicked()));

    KMenu *menu = new KMenu();
    menu->addTitle(i18n("Color Options"));
    m_configAndHistoryMenu = menu;
    QAction *act = m_configAndHistoryMenu->addAction(KIcon("edit-clear-history"), i18n("Clear History"));
    connect(act, SIGNAL(triggered(bool)), this, SLOT(clearHistory()));
    m_configAndHistoryMenu->addSeparator();

    // building the menu for default color string format.
    KMenu *m_colors_menu = new KMenu();
    m_colors_menu->addTitle(i18n("Default Format"));
    m_colors_format << "r, g, b" << "#RRGGBB" << "RRGGBB" << "#rrggbb" << "rrggbb";
    foreach (const QString& s, m_colors_format) {
      act = m_colors_menu->addAction(KIcon("draw-text"), s);
      act->setData(s);
    }

    m_colors_menu->addSeparator();
    act = m_colors_menu->addAction(KIcon("draw-text"), "Latex");
    act->setData("Latex");

    connect(m_colors_menu, SIGNAL(triggered(QAction*)), this, SLOT(setDefaultColorFormat(QAction*)));
    act = m_colors_menu->menuAction();
    act->setText(i18n("Default Color Format"));
    m_configAndHistoryMenu->addMenu(m_colors_menu);
}

void Kolourpicker::setDefaultColorFormat(QAction *act)
{
    if (!act) {
      return;
    }

    m_color_format = qvariant_cast<QString>(act->data());
}

Kolourpicker::~Kolourpicker()
{
    clearHistory(false);
    delete m_grabWidget;
    delete m_configAndHistoryMenu;
}

void Kolourpicker::init()
{
    configChanged();
}

void Kolourpicker::configChanged()
{
    // remove old entries, we are only interested in ones from the config now
    clearHistory(false);

    KConfigGroup cg = config();

    QList<QString> colorList = cg.readEntry("Colors", QList<QString>());
    m_color_format = cg.readEntry("ColorFormat", QString());

    Q_FOREACH (const QString &color, colorList) {
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

bool Kolourpicker::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_grabWidget && event->type() == QEvent::MouseButtonRelease) {
        m_grabWidget->removeEventFilter(this);
	m_grabWidget->hide();
        m_grabWidget->releaseMouse();
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        const QColor color = pickColor(me->globalPos());
        kDebug() << event->type() << me->globalPos() << color;
        addColor(color);
        colorActionTriggered(color);
    }
    return Plasma::Applet::eventFilter(watched, event);
}

QVariant Kolourpicker::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneChange) {
        QMetaObject::invokeMethod(this, "installFilter", Qt::QueuedConnection);
    }
    return Plasma::Applet::itemChange(change, value);
}

void Kolourpicker::grabClicked()
{
    if (m_grabWidget) {
	m_grabWidget->show();
	m_grabWidget->installEventFilter( this );
        m_grabWidget->grabMouse(Qt::CrossCursor);
    }
}

void Kolourpicker::historyClicked()
{
    m_configAndHistoryMenu->popup(QCursor::pos());
}

void Kolourpicker::colorActionTriggered(const QColor& color)
{
    if (!color.isValid()) {
        return;
    }

    QMimeData *mime = new QMimeData();
    mime->setColorData(color);

    QString text;
    /*
    converts the color according to the color format choosen by the user
    */
    if (m_color_format == "r, g, b") {
       text = QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue());
    } else if(m_color_format == "#RRGGBB") {
      text = color.name().toUpper();
    } else if(m_color_format == "RRGGBB") {
      text = color.name().toUpper().mid(1);
    } else if(m_color_format == "#rrggbb") {
      text = color.name();
    } else if(m_color_format == "rrggbb") {
      text = color.name().mid(1);
    } else if(m_color_format == "Latex") {
      text = toLatex(color);
    } else {
      text = QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue());
    }

    mime->setText(text);
    QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

QString Kolourpicker::toLatex(const QColor& color)
{
      qreal r = (qreal)color.red()/255;
      qreal g = (qreal)color.green()/255;
      qreal b = (qreal)color.blue()/255;

      return QString("\\definecolor{ColorName}{rgb}{%1,%2,%3}").arg(r,0,'f',2).arg(g,0,'f',2).arg(b,0,'f',2);
}

void Kolourpicker::colorActionTriggered(QAction *act)
{
    if (!act) {
        return;
    }

    QColor color = qvariant_cast<QColor>(act->data());
    QString text =  act->text().remove('&');

    if(text == i18n("Latex Color")) {
      text = toLatex(color);
    }

    QMimeData *mime = new QMimeData();
    mime->setColorData(color);
    mime->setText(text);
    QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

void Kolourpicker::clearHistory(bool save)
{
    m_configAndHistory->nativeWidget()->setIcon(ColorIcon(Qt::gray));
    QHash<QColor, QAction *>::ConstIterator it = m_menus.constBegin(), itEnd = m_menus.constEnd();
    for (; it != itEnd; ++it ) {
        m_configAndHistoryMenu->removeAction(*it);
        delete *it;
    }
    m_menus.clear();
    m_colors.clear();

    if (save) {
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
    if (it != m_menus.constEnd()) {
        return;
    }

    KMenu *newmenu = buildMenuForColor(color);
    QAction *act = newmenu->menuAction();
    ColorIcon colorIcon(color);
    act->setIcon(colorIcon);
    act->setText(QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    connect(newmenu, SIGNAL(triggered(QAction*)), this, SLOT(colorActionTriggered(QAction*)));
    m_configAndHistoryMenu->insertMenu(m_configAndHistoryMenu->actions().at(1), newmenu);
    m_configAndHistory->nativeWidget()->setIcon(colorIcon);
    m_menus.insert(color, act);
    m_colors.append(color.name());
    m_configAndHistory->setEnabled(true);
    if (save) {
        KConfigGroup cg = config();
        saveData(cg);
    }
}

void Kolourpicker::saveData(KConfigGroup &cg)
{
    cg.writeEntry("Colors", m_colors);
    cg.writeEntry("ColorFormat", m_color_format);
    emit configNeedsSaving();
}

#include "kolourpicker.moc"
