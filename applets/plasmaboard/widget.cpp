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
#include "BoardKey.h"
#include "AlphaNumKey.h"
#include "FuncKey.h"
#include "EnterKey.h"
#include "BackspaceKey.h"
#include "TabKey.h"
#include "ShiftKey.h"
#include "CapsKey.h"
#include "ArrowTopKey.h"
#include "ArrowBottomKey.h"
#include "ArrowLeftKey.h"
#include "ArrowRightKey.h"
#include <QPainter>
#include <QGraphicsSceneResizeEvent>
#include <QFile>
#include <plasma/theme.h>
#include "Helpers.h"

#define BACKSPACEKEY 0
#define TABKEY 1
#define ENTERKEY 2
#define CAPSKEY 3
#define SHIFT_L_KEY 4
#define SHIFT_R_KEY 5
#define CTLKEY 6
#define SUPER_L_KEY 7
#define ALT_L_KEY 8
#define SPACE 9
#define ALTGRKEY 10
#define SUPER_R_KEY 11

#define MENU 12
#define CONTROL_LEFT 13

#define HOMEKEY 16
#define ENDKEY 17
#define INSKEY 18
#define DELKEY 19
#define PGUPKEY 20
#define PGDOWNKEY 21

#define ARROWTOPKEY 	22
#define ARROWLEFTKEY	23
#define ARROWBOTTOMKEY	24
#define ARROWRIGHTKEY	25

QChar Helpers::mapXtoUTF8[0xffff+1];

PlasmaboardWidget::PlasmaboardWidget(QGraphicsWidget *parent)
    : Plasma::Containment(parent)
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

    m_tooltip = new Tooltip("");

    m_frame = new Plasma::FrameSvg();
    m_frame->setCacheAllRenderedFrames(false);
    m_frame->setImagePath("widgets/button");
    m_frame->setElementPrefix("normal");

    m_activeFrame = new Plasma::FrameSvg();
    m_activeFrame->setCacheAllRenderedFrames(false);
    m_activeFrame->setImagePath("widgets/button");
    m_activeFrame->setElementPrefix("active");

    m_engine = dataEngine("keystate");
    if(m_engine){
        m_engine -> connectAllSources(this);
    }

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}


PlasmaboardWidget::~PlasmaboardWidget()
{
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

    // DO I STILL NEED THIS STUFF BELOW???

    bool change = false;
    if( m_funcKeys[SHIFT_L_KEY]->toggled() || m_funcKeys[SHIFT_R_KEY]->toggled() ){
        Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Shift_L));
        m_funcKeys[SHIFT_L_KEY]->toggleOff();
        m_funcKeys[SHIFT_R_KEY]->toggleOff();
        m_isLevel2 = false;
        change = true;
    }
    if( m_funcKeys[ALTGRKEY]->toggled() ){
        Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_ISO_Level3_Shift));
        m_funcKeys[ALTGRKEY]->toggleOff();
        m_isAlternative = false;
        change = true;
    }

    if(change){
        relabelKeys();
    }

    Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Control_L));
    m_funcKeys[CTLKEY]->toggleOff();
    m_funcKeys[CONTROL_LEFT]->toggleOff();
    Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Meta_L));
    m_funcKeys[SUPER_L_KEY]->toggleOff();
    m_funcKeys[SUPER_R_KEY]->toggleOff();
    Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Alt_L));
    m_funcKeys[ALT_L_KEY]->toggleOff();

    clearTooltip();
}

void PlasmaboardWidget::clearAnything()
{
    if ( m_isLocked ) {
        //m_funcKeys[CAPSKEY]->sendKeycode();
    }
    clear();
}

void PlasmaboardWidget::clearTooltip()
{
    m_tooltip->hide();
}

FuncKey* PlasmaboardWidget::createFunctionKey(QPoint &point, QSize &size, QString action)
{

    if(action == "ALT"){
        FuncKey *k = new FuncKey(point, size, XK_Alt_L, QString(i18n("Alt")));
        m_altKeys << k;
        return k;
    }
    else if(action == "ALTGR"){
        FuncKey *k = new FuncKey(point, size, XK_Alt_L, QString( i18nc("The Alt Gr key on a keyboard", "Alt Gr")));
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
        FuncKey *k = new FuncKey(point, size, XK_Control_L, QString(i18nc("The Ctrl key on a keyboard", "Ctrl")));
        m_ctlKeys << k;
        return k;
    }
    else if(action == "CONTROLRIGHT"){
        FuncKey *k = new FuncKey(point, size, XK_Control_R, QString(i18nc("The Ctrl key on a keyboard", "Ctrl")));
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
        return new FuncKey(point, size, XK_space, QString());
    else if(action == "SUPERLEFT"){
        FuncKey *k = new FuncKey(point, size, XK_Super_L, QString( i18nc("The super (windows) key on a keyboard", "Super")));
        m_superKeys << k;
        return k;
    }
    else if(action == "TAB")
        return new TabKey(point, size);

    return new FuncKey(point, size, XK_Tab, QString(i18n("Tab")));
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
                    m_alphaKeys << new AlphaNumKey(currentPoint, currentSize, QVariant(m_xmlReader.attributes().value("code").toString()).toInt());
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
            clearTooltip();
            return;
        }

        Q_FOREACH(BoardKey* key, m_pressedList){
            if(key->contains(click)){
                return; // if mouse move is inside an already clicked button, we do nothing
            }
        }

        Q_FOREACH(BoardKey* key, m_keys){
            if(key->contains(click)){
                Q_FOREACH(BoardKey* clickedKey, m_pressedList){ // release all pressed keys
                    unpress(clickedKey);
                }
                press(key);
                return;
            }
        }
    }
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
}

/*bool PlasmaboardWidget::event ( QEvent * event )
{
    qDebug() << event->type();
    return QGraphicsWidget::event(event);
}*/

void PlasmaboardWidget::paint(QPainter *p,
                              const QStyleOptionGraphicsItem *option,
                              QWidget* widget)
{
    Q_UNUSED(widget);
    //Plasma::Containment::paint(p, option, widget);

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);
    p->setFont(QFont( Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).toString(), 200));

    if( boundingRect().contains(option->exposedRect) ){
        QRectF rect = option->exposedRect;
        Q_FOREACH(BoardKey *key, m_keys){
            if(key->intersects(rect)){
                key->paint(p);
            }
        }
        qDebug() << "Partially Painting!";
        return;
    }

    Q_FOREACH(BoardKey* key, m_keys){
        key->paint(p);
    }

    qDebug() << "Painting!";
}

void PlasmaboardWidget::press(BoardKey *key)
{
    key->pressed();
    key->setPixmap(getActiveFrame(key->size()));
    m_pressedList << key;    
    update(key->rect());
    setTooltip(key);
}

void PlasmaboardWidget::press(FuncKey *key)
{
    key->pressed();
    key->setPixmap(getActiveFrame(key->size()));
    m_pressedList << key;
    update(key->rect());
}

void PlasmaboardWidget::refreshKeys()
{
    double factor_x = size().width() / 10000;
    double factor_y = size().height() / 10000;

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
    key->released();
    key->setPixmap(getFrame(key->size()));
    m_pressedList.removeAll(key);
    update(key->rect());
    clearTooltip();
}

void PlasmaboardWidget::resetKeyboard()
{
    qDeleteAll(m_funcKeys);
    m_funcKeys.clear();

    qDeleteAll(m_alphaKeys);
    m_alphaKeys.clear();

    m_keys.clear();
    m_altKeys.clear();
    m_altgrKeys.clear();;
    m_capsKeys.clear();
    m_ctlKeys.clear();;
    m_shiftKeys.clear();        
    m_superKeys.clear();
}

void PlasmaboardWidget::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Q_UNUSED(event);
    refreshKeys();
}

void PlasmaboardWidget::setTooltip(BoardKey* key)
{
    QString label = key->label();
    if(label.size() > 0) {
        m_tooltip -> setText( key->label() );
        m_tooltip -> move( popupPosition( key->size() ) + key->position() );
        m_tooltip -> resize( key->size() );
        m_tooltip -> show();
    }
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
    key->unpressed();
    key->setPixmap(getFrame(key->size()));
    update(key->rect());
    m_pressedList.removeAll(key);
}

