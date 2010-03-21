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
    m_frame->setImagePath("widgets/button");
    m_frame->setElementPrefix("normal");

    m_activeFrame = new Plasma::FrameSvg();
    m_activeFrame->setImagePath("widgets/button");
    m_activeFrame->setElementPrefix("active");

    m_engine = dataEngine("keystate");
    if(m_engine){
        m_engine -> connectAllSources(this);
    }

     connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(refresh()));
}


PlasmaboardWidget::~PlasmaboardWidget()
{
    delete m_frame;
    delete m_activeFrame;
    delete m_tooltip;
    qDeleteAll(m_alphaKeys);
    qDeleteAll(m_funcKeys);
    qDeleteAll(m_frames);
    qDeleteAll(m_activeFrames);
}

void PlasmaboardWidget::clear()
{
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

    //clearTooltip();
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

void PlasmaboardWidget::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    if ( sourceName == "Shift" ){
	if ( data["Pressed"].toBool() ){
	    emit shiftKey(true);
        m_isLevel2 = true;
	}
	else{
	    emit shiftKey(false);
        m_isLevel2 = false;
	}
    }
    
    else if ( sourceName == "Caps Lock" ) {
	if ( data["Pressed"].toBool() )
        m_isLocked = true;
	else
        m_isLocked = false;
    }

    else if ( sourceName == "AltGr" ) {
	if ( data["Pressed"].toBool() ){
        m_isAlternative = true;
	    emit altGrKey(true);
	}
	else{
        m_isAlternative = false;
	    emit altGrKey(false);
	}
    }

    else if ( sourceName == "Alt" ) {
	if ( data["Pressed"].toBool() )
	    emit altKey(true);
	else
	    emit altKey(false);
    }

    else if ( sourceName == "Super" ) {
	if ( data["Pressed"].toBool() )
	    emit superKey(true);
	else
	    emit superKey(false);
    }

    else if ( sourceName == "Ctrl" ) {
	if ( data["Pressed"].toBool() )
	    emit controlKey(true);
	else
	    emit controlKey(false);
    }
    else if ( sourceName == "Menu" ) {
	if ( data["Pressed"].toBool() )
	    emit menuKey(true);
	else
	    emit menuKey(false);
    }
    relabelKeys();
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

void PlasmaboardWidget::initKeyboard()
{
    m_funcKeys << new FuncKey(QPoint(0,0), QSize(700,1000), XK_Escape, QString(i18n("Esc")));
    m_funcKeys << new FuncKey(QPoint(725,0), QSize(700,1000), XK_F1, QString(i18n("F1")));
    m_funcKeys << new FuncKey(QPoint(1450,0), QSize(700,1000), XK_F2, QString(i18n("F2")));
    m_funcKeys << new FuncKey(QPoint(2175,0), QSize(700,1000), XK_F3, QString(i18n("F3")));
    m_funcKeys << new FuncKey(QPoint(2900,0), QSize(700,1000), XK_F4, QString(i18n("F4")));
    m_funcKeys << new FuncKey(QPoint(3625,0), QSize(700,1000), XK_F5, QString(i18n("F5")));
    m_funcKeys << new FuncKey(QPoint(4350,0), QSize(700,1000), XK_F6, QString(i18n("F6")));
    m_funcKeys << new FuncKey(QPoint(5075,0), QSize(700,1000), XK_F7, QString(i18n("F7")));
    m_funcKeys << new FuncKey(QPoint(5800,0), QSize(700,1000), XK_F8, QString(i18n("F8")));
    m_funcKeys << new FuncKey(QPoint(6525,0), QSize(700,1000), XK_F9, QString(i18n("F9")));
    m_funcKeys << new FuncKey(QPoint(7250,0), QSize(700,1000), XK_F10, QString(i18n("F10")));
    m_funcKeys << new FuncKey(QPoint(7975,0), QSize(700,1000), XK_F11, QString(i18n("F11")));
    m_funcKeys << new FuncKey(QPoint(8700,0), QSize(700,1000), XK_F12, QString(i18n("F12")));

    m_alphaKeys << new AlphaNumKey(QPoint(2200,100), QSize(2000,2000), 42);
    m_alphaKeys << new AlphaNumKey(QPoint(4400,100), QSize(2000,2000), 43);
    m_alphaKeys << new AlphaNumKey(QPoint(1100,2200), QSize(4000,4000), 44);

    Q_FOREACH(BoardKey* key, m_alphaKeys){
        m_keys << key;
    }
    Q_FOREACH(BoardKey* key, m_funcKeys){
        m_keys << key;
    }

}

void PlasmaboardWidget::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{    
    if(m_pressedList.count() > 0){
        QPoint click = event->pos().toPoint();

        /*if(!boundingRect().contains(click)){
            Q_FOREACH(BoardKey* clickedKey, m_pressedList){ // release all pressed keys
                unpress(clickedKey);
            }
            return;
        }*/

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
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);
    p->setFont(QFont( Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).toString(), 200));

    if(option->exposedRect != boundingRect()){
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

void PlasmaboardWidget::refresh()
{
    qDeleteAll(m_frames);
    m_frames.clear();
    qDeleteAll(m_activeFrames);
    m_activeFrames.clear();

    double factor_x = size().width() / 10000;
    double factor_y = size().height() / 10000;

    Q_FOREACH(BoardKey* key, m_keys){
        key->updateDimensions(factor_x, factor_y);
        key->setPixmap(getFrame(key->size()));
    }
    update();
}

void PlasmaboardWidget::relabelKeys()
{
    foreach (AlphaNumKey* key, m_alphaKeys){
        key->switchKey(m_isLevel2, m_isAlternative, m_isLocked);
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
}

void PlasmaboardWidget::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    refresh();
}

void PlasmaboardWidget::setTooltip(BoardKey* key)
{
    m_tooltip -> setText( key->label() );
    m_tooltip -> move( popupPosition( key->size() ) + key->position() );
    m_tooltip -> resize( key->size() );
    m_tooltip -> show();
}

void PlasmaboardWidget::unpress(BoardKey *key)
{
    key->unpressed();
    key->setPixmap(getFrame(key->size()));
    update(key->rect());
    m_pressedList.removeAll(key);
}

