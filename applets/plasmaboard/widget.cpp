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

#include <QFile>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneResizeEvent>
#include <QTimer>
#include <QPainter>
#include <QSignalMapper>
#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/theme.h>

#include "Helpers.h"

#define REPEAT_TIMER 1500
#define STICKY_TIMER 500

QChar Helpers::mapXtoUTF8[0xffff+1];
int Helpers::keysymsPerKeycode;

PlasmaboardWidget::PlasmaboardWidget(Plasma::PopupApplet *parent)
    : QGraphicsWidget(parent),
      m_applet(parent)
{
    setPreferredSize(500, 200);
    setMinimumSize(200,100);
    setMaximumSize(20000,10000);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    Helpers::buildUp();
    m_isLevel2 = false;
    m_isAlternative = false;
    m_isLocked = false;
    m_isRepeating = false;

    m_tooltip = new Tooltip("");

    m_frame = new Plasma::FrameSvg();
    m_frame->setCacheAllRenderedFrames(false);
    m_frame->setImagePath("widgets/button");
    m_frame->setElementPrefix("normal");

    m_activeFrame = new Plasma::FrameSvg();
    m_activeFrame->setCacheAllRenderedFrames(false);
    m_activeFrame->setImagePath("widgets/button");
    m_activeFrame->setElementPrefix("pressed");

    m_engine = m_applet->dataEngine("keystate");
    if(m_engine){
        m_engine->connectAllSources(this);
    }

    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(stickyKey_Mapper(int)));

    m_repeatTimer = new QTimer(this);
    connect(m_repeatTimer, SIGNAL(timeout()), this, SLOT(repeatKeys()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}


PlasmaboardWidget::~PlasmaboardWidget()
{
    reset();
    delete m_frame;
    delete m_activeFrame;
    delete m_tooltip;
    qDeleteAll(m_keys);
    qDeleteAll(m_frames);
    qDeleteAll(m_activeFrames);
}

void PlasmaboardWidget::change(FuncKey *key, bool state)
{
    if(state)
        press(key);
    else
        unpress(key);
}

void PlasmaboardWidget::clear()
{
    Q_FOREACH(BoardKey* key, m_pressedList){
        key->unpressed();
    }

    clearTooltip();
}

void PlasmaboardWidget::clearTooltip()
{
    m_tooltip->hide();
}

FuncKey* PlasmaboardWidget::createFunctionKey(const QPoint &point, const QSize &size, const QString &action)
{

    if(action == "ALT"){
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Alt_L), QString(i18n("Alt")));
        m_altKeys << k;
        return k;
    }
    else if(action == "ALTGR"){
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_ISO_Level3_Shift), QString( i18nc("The Alt Gr key on a keyboard", "Alt Gr")));
        m_altgrKeys << k;
        return k;
    }
    else if(action == "BACKSPACE")
        return new BackspaceKey(point, size);
    else if(action == "CAPSLOCK"){
        CapsKey *k = new CapsKey(point, size);
        m_capsKeys << k;
        return k;
    }
    else if(action == "CONTROLLEFT"){
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Control_L), QString(i18nc("The Ctrl key on a keyboard", "Ctrl")));
        m_ctlKeys << k;
        return k;
    }
    else if(action == "CONTROLRIGHT"){
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Control_R), QString(i18nc("The Ctrl key on a keyboard", "Ctrl")));
        m_ctlKeys << k;
        return k;
    }
    else if(action == "ENTER")
        return new EnterKey(point, size);
    else if(action == "SHIFT") {
        ShiftKey *k = new ShiftKey(point, size);
        m_shiftKeys << k;
        return k;
    }
    else if(action == "SPACE")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_space), QString());
    else if(action == "SUPERLEFT"){
        StickyKey *k = new StickyKey(point, size, Helpers::keysymToKeycode(XK_Super_L), QString( i18nc("The super (windows) key on a keyboard", "Super")));
        m_superKeys << k;
        return k;
    }
    else if(action == "TAB")
        return new TabKey(point, size);
    else if(action == "ESCAPE")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Escape), QString(i18nc("The escape key on a keyboard", "Esc")));
    else if(action == "F1")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F1), QString(i18n("F1")));
    else if(action == "F2")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F2), QString(i18n("F2")));
    else if(action == "F3")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F3), QString(i18n("F3")));
    else if(action == "F4")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F4), QString(i18n("F4")));
    else if(action == "F5")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F5), QString(i18n("F5")));
    else if(action == "F6")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F6), QString(i18n("F6")));
    else if(action == "F7")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F7), QString(i18n("F7")));
    else if(action == "F8")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F8), QString(i18n("F8")));
    else if(action == "F9")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F9), QString(i18n("F9")));
    else if(action == "F10")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F10), QString(i18n("F10")));
    else if(action == "F11")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F11), QString(i18n("F11")));
    else if(action == "F12")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_F12), QString(i18n("F12")));
    else if(action == "PRINT")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Print), QString(i18nc("The print key on a keyboard", "Print")));
    else if(action == "NUM")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Num_Lock), QString(i18nc("The num key on a keyboard", "Num")));
    else if(action == "PAUSE")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Pause), QString(i18nc("The pause key on a keyboard", "Pause")));
    else if(action == "HOME")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Home), QString(i18nc("The home key on a keyboard", "Home")));
    else if(action == "DEL")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Delete), QString(i18nc("The delete key on a keyboard", "Del")));
    else if(action == "END")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_End), QString(i18nc("The end key on a keyboard", "End")));
    else if(action == "PAGEUP")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Page_Up), QString(i18nc("The page up key on a keyboard", "PgUp")));
    else if(action == "INSERT")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Insert), QString(i18nc("The insert key on a keyboard", "Ins")));
    else if(action == "PAGEDOWN")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_Page_Down), QString(i18nc("The page down key on a keyboard", "PgDn")));
    else if(action == "ARROWUP")
        return new ArrowTopKey(point, size);
    else if(action == "ARROWDOWN")
        return new ArrowBottomKey(point, size);
    else if(action == "ARROWLEFT")
        return new ArrowLeftKey(point, size);
    else if(action == "ARROWRIGHT")
        return new ArrowRightKey(point, size);
    else if(action == "KEYPADDIVIDE")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Divide), QString(i18nc("Divider on the keypad", "/")));
    else if(action == "KEYPADMULTIPLY")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Multiply), QString(i18nc("Multiplier on keypad", "*")));
    else if(action == "KEYPADADD")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Add), QString(i18nc("Plus sign Divider on the keypad", "+")));
    else if(action == "KEYPADSUBTRACT")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Subtract), QString(i18nc("Plus sign Divider on the keypad", "+")));
    else if(action == "KEYPADENTER")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Enter), QString(i18nc("Enter key on the keypad", "Enter")));
    else if(action == "KEYPADSEPARATOR")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_Separator), QString(i18nc("Seperator key on the keypad", ".")));
    else if(action == "KEYPAD1")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_1), QString(i18nc("1 key on the keypad", "1")));
    else if(action == "KEYPAD2")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_2), QString(i18nc("2 key on the keypad", "2")));
    else if(action == "KEYPAD3")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_3), QString(i18nc("3 key on the keypad", "3")));
    else if(action == "KEYPAD4")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_4), QString(i18nc("4 key on the keypad", "4")));
    else if(action == "KEYPAD5")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_5), QString(i18nc("5 key on the keypad", "5")));
    else if(action == "KEYPAD6")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_6), QString(i18nc("6 key on the keypad", "6")));
    else if(action == "KEYPAD7")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_7), QString(i18nc("7 key on the keypad", "7")));
    else if(action == "KEYPAD8")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_8), QString(i18nc("8 key on the keypad", "8")));
    else if(action == "KEYPAD9")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_9), QString(i18nc("9 key on the keypad", "9")));
    else if(action == "KEYPAD0")
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_KP_0), QString(i18nc("0 key on the keypad", "0")));
    else if(action == "SWITCH"){
        SwitchKey* key = new SwitchKey(point, size, Helpers::keysymToKeycode(XK_VoidSymbol), this);
        m_switchKeys << key;
        return key;
    }
    else
        return new FuncKey(point, size, Helpers::keysymToKeycode(XK_space), QString("Unkown"));
}

void PlasmaboardWidget::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    bool state = data["Pressed"].toBool();

    if ( sourceName == "Shift" ){
        Q_FOREACH(FuncKey* key, m_shiftKeys){
            change(key, state);
        }
        m_isLevel2 = state;
        relabelKeys();
    }
    
    else if ( sourceName == "Caps Lock" ) {
        Q_FOREACH(FuncKey* key, m_capsKeys){
            change(key, state);
        }
        m_isLocked = state;
        relabelKeys();
    }

    else if ( sourceName == "AltGr" ) {
        Q_FOREACH(FuncKey* key, m_altgrKeys){
            change(key, state);
        }
        m_isAlternative = state;
        relabelKeys();
    }

    else if ( sourceName == "Alt" ) {
        Q_FOREACH(FuncKey* key, m_altKeys){
            change(key, state);
        }
    }

    else if ( sourceName == "Super" ) {
        Q_FOREACH(FuncKey* key, m_superKeys){
            change(key, state);
        }
    }

    else if ( sourceName == "Ctrl" ) {
        Q_FOREACH(FuncKey* key, m_ctlKeys){
            change(key, state);
        }
    }
    else if ( sourceName == "Menu" ) {

    }    
}

void PlasmaboardWidget::deleteKeys()
{
    qDeleteAll(m_funcKeys);
    m_funcKeys.clear();

    qDeleteAll(m_alphaKeys);
    m_alphaKeys.clear();

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

QPixmap *PlasmaboardWidget::getActiveFrame(const QSize &size)
{
    QPixmap *pixmap;
    if(!m_activeFrames.contains(size)){
        m_activeFrame->resizeFrame(size);
        pixmap = new QPixmap(m_activeFrame->framePixmap());
        m_activeFrames[size] = pixmap;
    }
    else{
        pixmap = m_activeFrames[size];
    }
    return pixmap;

}

QPixmap *PlasmaboardWidget::getFrame(const QSize &size)
{
    QPixmap *pixmap;
    if(!m_frames.contains(size)){
        m_frame->resizeFrame(size);
        pixmap = new QPixmap(m_frame->framePixmap());
        m_frames[size] = pixmap;
    }
    else{
        pixmap = m_frames[size];
    }
    return pixmap;
}

void PlasmaboardWidget::initKeyboard(const QString &file)
{
    int spacing = 100;
    int rowHeight = 100;
    int rowWidth = 100;
    QPoint currentPoint = QPoint(0,0);
    QSize currentSize = QSize(100,100);
    int currentWidth = 100;
    int currentHeight;

    QFile* fileP = new QFile(file);
    fileP->open(QIODevice::ReadOnly | QIODevice::Text);

    m_xmlReader.setDevice(fileP);

    // reading in header information
    if (m_xmlReader.readNextStartElement()) {       
        if (m_xmlReader.name() == "keyboard"){
            spacing = QVariant(m_xmlReader.attributes().value("spacing").toString()).toInt();

        }
        else {
            m_xmlReader.raiseError(i18n("Missing keyboard tag"));
        }
    }

    // building up layout
    while(!m_xmlReader.atEnd()) {
        m_xmlReader.readNextStartElement();

        if (m_xmlReader.name() == "row"){
            rowHeight = QVariant(m_xmlReader.attributes().value("height").toString()).toInt();
            rowWidth = QVariant(m_xmlReader.attributes().value("width").toString()).toInt();

            while(m_xmlReader.readNextStartElement()){

                currentPoint += QPoint(spacing, 0);

                if(m_xmlReader.attributes().hasAttribute("width")){
                    currentWidth = QVariant(m_xmlReader.attributes().value("width").toString()).toInt();
                }
                else {
                    currentWidth = rowWidth;
                }

                if(m_xmlReader.attributes().hasAttribute("height")){
                    currentHeight = QVariant(m_xmlReader.attributes().value("height").toString()).toInt();
                }
                else {
                    currentHeight = rowHeight;
                }

                currentSize = QSize(currentWidth, currentHeight);

                if(m_xmlReader.name() == "key"){
                    if(m_xmlReader.attributes().hasAttribute("alt")){
                        DualKey* key;
                        if(m_xmlReader.attributes().hasAttribute("altshifted")){
                            key = new DualKey(currentPoint, currentSize, QVariant(m_xmlReader.attributes().value("code").toString()).toInt(), m_xmlReader.attributes().value("alt").toString(), m_xmlReader.attributes().value("altshifted").toString());
                        }
                        else{
                            key = new DualKey(currentPoint, currentSize, QVariant(m_xmlReader.attributes().value("code").toString()).toInt(), m_xmlReader.attributes().value("alt").toString());
                        }

                        m_alphaKeys << key;
                        m_dualKeys << key;
                    }
                    else {
                        m_alphaKeys << new AlphaNumKey(currentPoint, currentSize, QVariant(m_xmlReader.attributes().value("code").toString()).toInt());
                    }
                }
                else if(m_xmlReader.name() == "fkey"){
                    m_funcKeys << createFunctionKey(currentPoint, currentSize, m_xmlReader.attributes().value("action").toString());
                }

                m_xmlReader.skipCurrentElement();
                currentPoint += QPoint(currentWidth, 0);

            }

            currentPoint = QPoint(0, currentPoint.y() + rowHeight + spacing);
        }

    }

    Q_FOREACH(BoardKey* key, m_alphaKeys){
        m_keys << key;
    }
    Q_FOREACH(BoardKey* key, m_funcKeys){
        m_keys << key;
    }

    delete fileP;
}

void PlasmaboardWidget::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{    
    if(m_pressedList.count() > 0){ // as long no key is pressed, we don't care about mouse moves
        QPoint click = event->pos().toPoint();

        if(!boundingRect().contains(click)){ // if mouse is moved outside the widget...
            Q_FOREACH(BoardKey* clickedKey, m_pressedList){  // ... we unpress all pressed keys
                unpress(clickedKey);
            }            
            return;
        }

        Q_FOREACH(BoardKey* key, m_pressedList){
            if(key->contains(click)){
                return; // if mouse move is inside an already clicked button, we do nothing
            }
        }

        Q_FOREACH(BoardKey* key, m_keys){
            if(m_isRepeating){
                Q_FOREACH(BoardKey* clickedKey, m_pressedList){ // release all pressed keys
                    clickedKey->released();
                }
                m_isRepeating = false;
            }

            if(key->contains(click)){
                Q_FOREACH(BoardKey* clickedKey, m_pressedList){ // release all pressed keys
                    unpress(clickedKey);
                }                
                press(key);
                return;
            }
        }
    }

    QGraphicsWidget::mouseMoveEvent(event);
}

void PlasmaboardWidget::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QPoint click = event->pos().toPoint();
    Q_FOREACH(BoardKey* key, m_keys){
        if(key->contains(click)){
            press(key);
            return;
        }
    }
    QGraphicsWidget::mousePressEvent(event);
}

void PlasmaboardWidget::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    QPoint click = event->pos().toPoint();
    Q_FOREACH(BoardKey* key, m_pressedList){
        if(key->contains(click)){
            release(key);
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
    qDebug() << "Painting: " << option->exposedRect;
    //Plasma::Containment::paint(p, option, widget);

    p->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));

    QRectF rect = option->exposedRect;
    Q_FOREACH(BoardKey *key, m_keys){
        if(key->intersects(rect)){
            key->paint(p);
        }
    }
}

void PlasmaboardWidget::press(BoardKey *key)
{
    key->pressed();
    key->setPixmap(getActiveFrame(key->size()));
    m_pressedList << key;    
    update(key->rect());
    setTooltip(key);
    m_repeatTimer->start(REPEAT_TIMER);
}

void PlasmaboardWidget::press(FuncKey *key)
{
    key->setPixmap(getActiveFrame(key->size()));
    m_pressedList << key;
    update(key->rect());
}

void PlasmaboardWidget::refreshKeys()
{
    double factor_x = size().width() / 10048;
    double factor_y = size().height() / 10002;

    Q_FOREACH(BoardKey* key, m_keys){
        key->updateDimensions(factor_x, factor_y);
        key->setPixmap(getFrame(key->size()));
    }
}

void PlasmaboardWidget::relabelKeys()
{
    foreach (AlphaNumKey* key, m_alphaKeys){
        key->switchKey(m_isLevel2, m_isAlternative, m_isLocked);
        update(key->rect());
    }
}

void PlasmaboardWidget::release(BoardKey *key)
{    
    key->released(); // trigger X-unpress event done by key
    m_pressedList.removeAll(key);    
    clearTooltip(); // remove displayed tooltip
    if(m_alphaKeys.contains((AlphaNumKey*) key)){
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
    Q_FOREACH(BoardKey* key, m_pressedList){
        key->pressRepeated();
    }
    m_isRepeating = true;
}

void PlasmaboardWidget::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Q_UNUSED(event);
    refreshKeys();
}

void PlasmaboardWidget::reset(){

    Q_FOREACH(BoardKey* key, m_pressedList){
        unpress(key);
    }

    Q_FOREACH(StickyKey* key, m_altKeys){
        key->reset();
    }

    Q_FOREACH(StickyKey* key, m_altgrKeys){
        key->reset();
    }

    Q_FOREACH(StickyKey* key, m_ctlKeys){
        key->reset();
    }

    Q_FOREACH(StickyKey* key, m_shiftKeys){
        key->reset();
    }

    Q_FOREACH(StickyKey* key, m_superKeys){
        key->reset();
    }

    Q_FOREACH(SwitchKey* key, m_switchKeys){
        key->reset();
        unpress(key);
    }
}

void PlasmaboardWidget::setTooltip(BoardKey* key)
{
    QString label = key->label();
    if(label.size() > 0) {
        m_tooltip -> setText( key->label() );
        m_tooltip -> resize( key->size()*2 );

        Plasma::Containment *c = m_applet->containment();
        if(c){
            Plasma::Corona *corona = c->corona();
            if(corona){
                m_tooltip -> move( corona->popupPosition(this, key->size()*2, Qt::AlignLeft) + key->position() - QPoint(key->size().width()/2, 0) );
            }
        }

        m_tooltip -> show();
    }
}

void PlasmaboardWidget::stickyKey_Mapper(int id)
{
    BoardKey* key = m_stickyKeys[id];
    key->setPixmap(getFrame(key->size()));
    update(key->rect());

    delete (m_signalMapper->mapping(id)); // delete the QTimer
    m_stickyKeys.remove(id);
}

void PlasmaboardWidget::switchAlternative(bool alt){
    Q_FOREACH(DualKey* key, m_dualKeys){
        key->setAlternative(alt);
    }
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
    key->unpressed();
    key->setPixmap(getFrame(key->size()));
    update(key->rect());
    m_pressedList.removeAll(key);
    m_repeatTimer->stop();
}

