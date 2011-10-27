/*/***************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>    *
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


#include "widget.h"

#include "AlphaNumKey.h"
#include "ArrowBottomKey.h"
#include "ArrowLeftKey.h"
#include "ArrowRightKey.h"
#include "ArrowTopKey.h"
#include "BackspaceKey.h"
#include "BoardKey.h"
#include "CapsKey.h"
#include "DualKey.h"
#include "EnterKey.h"
#include "FuncKey.h"
#include "ShiftKey.h"
#include "StickyKey.h"
#include "SwitchKey.h"
#include "TabKey.h"

#include <QDBusConnection>
#include <QFile>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QSignalMapper>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/theme.h>

#include "Helpers.h"

static const int REPEAT_TIMER = 1000;
static const int STICKY_TIMER = 50;
static const int TOOLTIP_SHOW_DELAY = 100;

PlasmaboardWidget::PlasmaboardWidget(Plasma::PopupApplet *parent)
    : QGraphicsWidget(parent),
      m_applet(parent),
      m_isAlternative(false),
      m_isLevel2(false),
      m_isLocked(false),
      m_isRepeating(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_tooltip = new Tooltip();

    m_frameSvg = new Plasma::FrameSvg();
    m_frameSvg->setCacheAllRenderedFrames(true);
    m_frameSvg->setImagePath("widgets/button");

    m_engine = m_applet->dataEngine("keystate");
    if (m_engine) {
        m_engine->connectAllSources(this);
    }

    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(stickyKey_Mapper(int)));

    m_repeatTimer = new QTimer(this);
    connect(m_repeatTimer, SIGNAL(timeout()), this, SLOT(repeatKeys()));

    m_delayedToolTipShow = new QTimer(this);
    m_delayedToolTipShow->setSingleShot(true);
    m_delayedToolTipShow->setInterval(TOOLTIP_SHOW_DELAY);
    connect(m_delayedToolTipShow, SIGNAL(timeout()), this, SLOT(showToolTip()));

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts", "currentLayoutChanged", this, SLOT(relabelKeys()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}


PlasmaboardWidget::~PlasmaboardWidget()
{
    reset();
    delete m_frameSvg;
    delete m_tooltip;
    qDeleteAll(m_keys);
    qDeleteAll(m_frames);
    qDeleteAll(m_activeFrames);
}

template<typename T>
void PlasmaboardWidget::setKeysState(const QList<T> &keys, const StateActions &actions)
{
    foreach (const T &key, keys) {
        if (actions & Unpress) {
            unpress(key);
        }

        if (actions & Press) {
            press(key, actions & ExternalEvent);
        }

        if (actions & Reset) {
            StickyKey *skey = dynamic_cast<StickyKey *>(key);
            if (!skey || (skey->isToggled() && !skey->isPersistent())) {
                key->reset();
                unpress(key);
            }
        }

        if (actions & Release) {
            key->released();
        }
    }
}

void PlasmaboardWidget::clearTooltip()
{
    m_delayedToolTipShow->stop();
    m_tooltip->hide();
}

FuncKey* PlasmaboardWidget::createFunctionKey(const QPoint &point, const QSize &size, const QString &action)
{
    if (action == "BACKSPACE") {
        return new BackspaceKey(point, size);
    } else if (action == "ENTER") {
        return new EnterKey(point, size);
    } else if (action == "SPACE") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_space), QString());
    } else if (action == "TAB") {
        return new TabKey(point, size);
    } else if (action == "ESCAPE") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Escape), QString(i18nc("The escape key on a keyboard", "Esc")));
    } else if (action == "F1") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F1), QString(i18n("F1")));
    } else if (action == "F2") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F2), QString(i18n("F2")));
    } else if (action == "F3") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F3), QString(i18n("F3")));
    } else if (action == "F4") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F4), QString(i18n("F4")));
    } else if (action == "F5") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F5), QString(i18n("F5")));
    } else if (action == "F6") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F6), QString(i18n("F6")));
    } else if (action == "F7") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F7), QString(i18n("F7")));
    } else if (action == "F8") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F8), QString(i18n("F8")));
    } else if (action == "F9") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F9), QString(i18n("F9")));
    } else if (action == "F10") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F10), QString(i18n("F10")));
    } else if (action == "F11") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F11), QString(i18n("F11")));
    } else if (action == "F12") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F12), QString(i18n("F12")));
    } else if (action == "PRINT") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Print), QString(i18nc("The print key on a keyboard", "Print")));
    } else if (action == "NUM") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Num_Lock), QString(i18nc("The num key on a keyboard", "Num")));
    } else if (action == "PAUSE") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Pause), QString(i18nc("The pause key on a keyboard", "Pause")));
    } else if (action == "HOME") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Home), QString(i18nc("The home key on a keyboard", "Home")));
    } else if (action == "DEL") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Delete), QString(i18nc("The delete key on a keyboard", "Del")));
    } else if (action == "END") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_End), QString(i18nc("The end key on a keyboard", "End")));
    } else if (action == "PAGEUP") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Page_Up), QString(i18nc("The page up key on a keyboard", "PgUp")));
    } else if (action == "INSERT") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Insert), QString(i18nc("The insert key on a keyboard", "Ins")));
    } else if (action == "PAGEDOWN") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Page_Down), QString(i18nc("The page down key on a keyboard", "PgDn")));
    } else if (action == "ARROWUP") {
        return new ArrowTopKey(point, size);
    } else if (action == "ARROWDOWN") {
        return new ArrowBottomKey(point, size);
    } else if (action == "ARROWLEFT") {
        return new ArrowLeftKey(point, size);
    } else if (action == "ARROWRIGHT") {
        return new ArrowRightKey(point, size);
    } else if (action == "KEYPADDIVIDE") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Divide), QString(i18nc("Divider on the keypad", "/")));
    } else if (action == "KEYPADMULTIPLY") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Multiply), QString(i18nc("Multiplier on keypad", "*")));
    } else if (action == "KEYPADADD") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Add), QString(i18nc("Plus sign Divider on the keypad", "+")));
    } else if (action == "KEYPADSUBTRACT") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Subtract), QString(i18nc("Minus sign on the keypad", "-")));
    } else if (action == "KEYPADENTER") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Enter), QString(i18nc("Enter key on the keypad", "Enter")));
    } else if (action == "KEYPADSEPARATOR") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Separator), QString(i18nc("Separator key on the keypad", ".")));
    } else if (action == "KEYPAD1") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_1), QString(i18nc("1 key on the keypad", "1")));
    } else if (action == "KEYPAD2") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_2), QString(i18nc("2 key on the keypad", "2")));
    } else if (action == "KEYPAD3") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_3), QString(i18nc("3 key on the keypad", "3")));
    } else if (action == "KEYPAD4") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_4), QString(i18nc("4 key on the keypad", "4")));
    } else if (action == "KEYPAD5") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_5), QString(i18nc("5 key on the keypad", "5")));
    } else if (action == "KEYPAD6") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_6), QString(i18nc("6 key on the keypad", "6")));
    } else if (action == "KEYPAD7") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_7), QString(i18nc("7 key on the keypad", "7")));
    } else if (action == "KEYPAD8") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_8), QString(i18nc("8 key on the keypad", "8")));
    } else if (action == "KEYPAD9") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_9), QString(i18nc("9 key on the keypad", "9")));
    } else if (action == "KEYPAD0") {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_0), QString(i18nc("0 key on the keypad", "0")));
    } else {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_space), QString(i18n("Unknown")));
    }
}

FuncKey* PlasmaboardWidget::createStickyKey(const QPoint &point, const QSize &size, const QString &action)
{
    if (action == "ALT") {
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Alt_L), QString(i18n("Alt")));
        m_altKeys << k;
        return k;
    } else if (action == "ALTGR") {
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_ISO_Level3_Shift), QString(i18nc("The Alt Gr key on a keyboard", "Alt Gr")));
        m_altgrKeys << k;
        return k;
    } else if (action == "CAPSLOCK") {
        CapsKey *k = new CapsKey(point, size);
        m_capsKeys << k;
        return k;
    } else if (action == "CONTROLLEFT") {
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Control_L), QString(i18nc("The Ctrl key on a keyboard", "Ctrl")));
        m_ctlKeys << k;
        return k;
    } else if (action == "CONTROLRIGHT") {
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Control_R), QString(i18nc("The Ctrl key on a keyboard", "Ctrl")));
        m_ctlKeys << k;
        return k;
    } else if (action == "SHIFT") {
        ShiftKey *k = new ShiftKey(point, size);
        m_shiftKeys << k;
        return k;
    } else if (action == "SUPERLEFT") {
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Super_L), QString(i18nc("The super (windows) key on a keyboard", "Super")));
        m_superKeys << k;
        return k;
    } else if (action == "SWITCH") {
        SwitchKey* key = new SwitchKey(point, size, Helpers::keysymToKeycode(XK_VoidSymbol), this);
        m_switchKeys << key;
        return key;
    } else {
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_space), QString("Unkown"));
    }
}

void PlasmaboardWidget::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    const bool state = data["Pressed"].toBool();
    const StateActions actions = (state ? Press : Unpress) | ExternalEvent;

    if (sourceName == "Shift") {
        setKeysState<StickyKey *>(m_shiftKeys, actions);
        m_isLevel2 = state;
        relabelKeys();
    } else if (sourceName == "Caps Lock") {
        setKeysState<FuncKey *>(m_capsKeys, actions);
        m_isLocked = state;
        relabelKeys();
    } else if (sourceName == "AltGr") {
        setKeysState<StickyKey *>(m_altgrKeys, actions);
        m_isAlternative = state;
        relabelKeys();
    } else if (sourceName == "Alt") {
        setKeysState<StickyKey *>(m_altgrKeys, actions);
    } else if (sourceName == "Super") {
        setKeysState<StickyKey *>(m_superKeys, actions);
    } else if (sourceName == "Ctrl") {
        setKeysState<StickyKey *>(m_ctlKeys, actions);
    } else if (sourceName == "Menu") {

    }
}

void PlasmaboardWidget::deleteKeys()
{
    qDeleteAll(m_keys);

    m_alphaKeys.clear();
    m_funcKeys.clear();
    m_specialKeys.clear();

    m_keys.clear();
    m_altKeys.clear();
    m_altgrKeys.clear();
    m_capsKeys.clear();
    m_ctlKeys.clear();
    m_dualKeys.clear();
    m_shiftKeys.clear();
    m_switchKeys.clear();
    m_superKeys.clear();
}

QPixmap *PlasmaboardWidget::background(BackgroundState state, const QSize &size)
{
    const bool normal = state == NormalBackground;
    QPixmap *pixmap = normal ? m_frames.value(size) : m_activeFrames.value(size);
    if (!pixmap) {
        m_frameSvg->setElementPrefix(normal ? "normal" : "pressed");
        m_frameSvg->resizeFrame(size);
        pixmap = new QPixmap(m_frameSvg->framePixmap());
        if (normal) {
            m_frames[size] = pixmap;
        } else {
            m_activeFrames[size] = pixmap;
        }
    }

    return pixmap;
}

void PlasmaboardWidget::initKeyboard(const QString &file)
{
    int spacing = 100;
    int rowHeight = 100;
    int rowWidth = 100;
    QPoint currentPoint = QPoint(0, 0);
    QSize currentSize = QSize(100, 100);
    int currentWidth = 100;
    int currentHeight;

    QFile fileP(file);
    fileP.open(QIODevice::ReadOnly | QIODevice::Text);

    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&fileP);

    // reading in header information
    if (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "keyboard") {
            spacing = QVariant(xmlReader.attributes().value("spacing").toString()).toInt();
        } else {
            xmlReader.raiseError(i18n("Missing keyboard tag"));
        }
    }

    // building up layout
    while (!xmlReader.atEnd()) {
        xmlReader.readNextStartElement();

        if (xmlReader.name() == "row") {
            rowHeight = QVariant(xmlReader.attributes().value("height").toString()).toInt();
            rowWidth = QVariant(xmlReader.attributes().value("width").toString()).toInt();

            while (xmlReader.readNextStartElement()) {

                currentPoint += QPoint(spacing, 0);

                if (xmlReader.attributes().hasAttribute("width")) {
                    currentWidth = QVariant(xmlReader.attributes().value("width").toString()).toInt();
                } else {
                    currentWidth = rowWidth;
                }

                if (xmlReader.attributes().hasAttribute("height")) {
                    currentHeight = QVariant(xmlReader.attributes().value("height").toString()).toInt();
                } else {
                    currentHeight = rowHeight;
                }

                currentSize = QSize(currentWidth, currentHeight);

                if (xmlReader.name() == "key") {
                    if (xmlReader.attributes().hasAttribute("alt")) {
                        DualKey* key;
                        if (xmlReader.attributes().hasAttribute("altshifted")) {
                            key = new DualKey(currentPoint, currentSize, QVariant(xmlReader.attributes().value("code").toString()).toInt(), xmlReader.attributes().value("alt").toString(), xmlReader.attributes().value("altshifted").toString());
                        } else {
                            key = new DualKey(currentPoint, currentSize, QVariant(xmlReader.attributes().value("code").toString()).toInt(), xmlReader.attributes().value("alt").toString());
                        }

                        m_alphaKeys << key;
                        m_dualKeys << key;
                    } else {
                        m_alphaKeys << new AlphaNumKey(currentPoint, currentSize, QVariant(xmlReader.attributes().value("code").toString()).toInt());
                    }
                } else if (xmlReader.name() == "fkey") {
                    m_specialKeys << createFunctionKey(currentPoint, currentSize, xmlReader.attributes().value("action").toString());
                } else if (xmlReader.name() == "skey") {
                    m_funcKeys << createStickyKey(currentPoint, currentSize, xmlReader.attributes().value("action").toString());
                }

                xmlReader.skipCurrentElement();
                currentPoint += QPoint(currentWidth, 0);

            }

            currentPoint = QPoint(0, currentPoint.y() + rowHeight + spacing);
        }
    }

    m_keyboardSize = currentSize;

    foreach (BoardKey * key, m_alphaKeys) {
        m_keys << key;
    }

    foreach (BoardKey * key, m_funcKeys) {
        m_keys << key;
    }

    foreach (BoardKey * key, m_specialKeys) {
        m_keys << key;
    }
}

void PlasmaboardWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_pressedList.count() > 0) { // as long no key is pressed, we don't care about mouse moves
        QPoint click = event->pos().toPoint();

        if (!boundingRect().contains(click)) { // if mouse is moved outside the widget...
            // ... we unpress all pressed keys
            setKeysState<BoardKey *>(m_pressedList, Unpress);
            return;
        }

        foreach (BoardKey * key, m_pressedList) {
            if (key->contains(click)) {
                return; // if mouse move is inside an already clicked button, we do nothing
            }
        }

        if (m_isRepeating) {
            // release all pressed keys
            setKeysState<BoardKey *>(m_pressedList, Release);
            m_isRepeating = false;
        }

        foreach (BoardKey * key, m_keys) {
            if (key->contains(click)) {
                // ... we unpress all pressed keys
                setKeysState<BoardKey *>(m_pressedList, Unpress);
                press(key);
                return;
            }
        }
    }

    QGraphicsWidget::mouseMoveEvent(event);
}

void PlasmaboardWidget::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    const QPoint click = event->pos().toPoint();

    foreach (BoardKey *key, m_keys) {
        if (key->contains(click)) {
            press(key);
            return;
        }
    }

    QGraphicsWidget::mousePressEvent(event);
}

void PlasmaboardWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    const QPoint click = event->pos().toPoint();

    foreach (BoardKey *key, m_pressedList) {
        if (key->contains(click)) {
            StickyKey *skey = dynamic_cast<StickyKey *>(key);
            if (skey) {
                if (skey->isPersistent()) {
                    skey->setPersistent(false);
                } else if (skey->isToggled()) {
                    skey->setPersistent(true);
                }

                if (!skey->isPersistent()) {
                    release(key);
                }
            } else {
                release(key);
            }
            return;
        }
    }

    QGraphicsWidget::mouseReleaseEvent(event);
}

void PlasmaboardWidget::paint(QPainter *p,
                              const QStyleOptionGraphicsItem *option,
                              QWidget* widget)
{
    Q_UNUSED(widget);
    //qDebug() << "Painting: " << option->exposedRect;
    //Plasma::Containment::paint(p, option, widget);

    p->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));

    QRectF rect = option->exposedRect;
    foreach (BoardKey * key, m_keys) {
        if (key->intersects(rect)) {
            key->paint(p);
        }
    }
}

void PlasmaboardWidget::press(BoardKey *key, bool externalEvent)
{
    if (!externalEvent) {
        key->pressed();
    }

    if (key->setPixmap(background(ActiveBackground, key->size()))) {
        update(key->rect());
    }

    if (!externalEvent) {
        m_pressedList.append(key);

        if (key->repeats()) {
            setTooltip(key);
            m_repeatTimer->start(REPEAT_TIMER);
        }
    }
}

void PlasmaboardWidget::refreshKeys()
{
    double factor_x = size().width() / 10048;
    double factor_y = size().height() / 10002;

    foreach (BoardKey * key, m_keys) {
        key->updateDimensions(factor_x, factor_y);
        key->setPixmap(background(NormalBackground, key->size()));
    }

    update();
}

void PlasmaboardWidget::relabelKeys()
{
    Helpers::refreshXkbState();
    foreach (AlphaNumKey * key, m_alphaKeys) {
        key->switchKey(m_isLevel2, m_isAlternative, m_isLocked);
        update(key->rect());
    }
}

void PlasmaboardWidget::release(BoardKey *key)
{
    key->released(); // trigger X-unpress event done by key
    m_pressedList.removeAll(key);
    clearTooltip(); // remove displayed tooltip
    if (m_alphaKeys.contains((AlphaNumKey*) key) || m_specialKeys.contains((FuncKey*) key)) {
        reset();
    }
    m_repeatTimer->stop();

    int id = qrand();
    m_stickyKeys[id] = key;

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(timer, id);
    timer->start(STICKY_TIMER);
}

void PlasmaboardWidget::repeatKeys()
{
    foreach (BoardKey *key, m_pressedList) {
        key->pressRepeated();
    }

    m_isRepeating = true;
}

void PlasmaboardWidget::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Q_UNUSED(event);
    refreshKeys();
}

void PlasmaboardWidget::reset()
{
    setKeysState<BoardKey *>(m_pressedList, Unpress);
    setKeysState<StickyKey *>(m_altKeys, Reset);
    setKeysState<StickyKey *>(m_altgrKeys, Reset);
    setKeysState<StickyKey *>(m_ctlKeys, Reset);
    setKeysState<StickyKey *>(m_shiftKeys, Reset);
    setKeysState<StickyKey *>(m_superKeys, Reset);
    setKeysState<SwitchKey *>(m_switchKeys, Reset);
}

void PlasmaboardWidget::setTooltip(BoardKey* key)
{
    QString label = key->label();
    if (label.size() > 0) {
        m_tooltip->setText(key->label());
        m_tooltip->resize(key->size() * 2);

        Plasma::Containment *c = m_applet->containment();
        if (c) {
            Plasma::Corona *corona = c->corona();
            if (corona) {
                if (m_applet->location() == Plasma::TopEdge) {
                    m_tooltip->move(corona->popupPosition(this, key->size() * 2, Qt::AlignLeft) + QPoint(key->position().x() - key->size().width() / 2, 0) - QPoint(0, size().height() - key->position().y() - key->size().height()));
                } else {
                    m_tooltip->move(corona->popupPosition(this, key->size() * 2, Qt::AlignLeft) + key->position() - QPoint(key->size().width() / 2, 0));
                }
            }
        }

        if (!m_tooltip->isVisible()) {
            m_delayedToolTipShow->start();
        }
    }
}

void PlasmaboardWidget::showToolTip()
{
    m_tooltip->show();
    m_tooltip->raise();
}

QSizeF PlasmaboardWidget::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
    if (which == Qt::MinimumSize) {
        return QSizeF(300, 100);
    } else if (which == Qt::PreferredSize) {
        if (!m_keyboardSize.isEmpty()) {
            return m_keyboardSize;
        } else {
            return QSizeF(800, 230);
        }
    } else if (which == Qt::MaximumSize) {
        return constraint;
    } else {
        return QGraphicsWidget::sizeHint(which, constraint);
    }

}


void PlasmaboardWidget::stickyKey_Mapper(int id)
{
    BoardKey* key = m_stickyKeys[id];
    if (key->setPixmap(background(NormalBackground, key->size()))) {
        update(key->rect());
    }

    delete(m_signalMapper->mapping(id));  // delete the QTimer
    m_stickyKeys.remove(id);
}

void PlasmaboardWidget::switchAlternative(bool alt)
{
    foreach (DualKey *key, m_dualKeys) {
        key->setAlternative(alt);
    }

    Helpers::flushPendingKeycodeMappingChanges();
    relabelKeys();
}

void PlasmaboardWidget::themeChanged()
{
    qDeleteAll(m_frames);
    m_frames.clear();
    qDeleteAll(m_activeFrames);
    m_activeFrames.clear();

    refreshKeys();
}

void PlasmaboardWidget::unpress(BoardKey *key)
{
    clearTooltip();
    if (key->setPixmap(background(NormalBackground, key->size()))) {
        update(key->rect());
    }
    m_pressedList.removeAll(key);
    m_repeatTimer->stop();
}

#include "widget.moc"


