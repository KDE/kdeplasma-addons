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
#include <QGraphicsGridLayout>

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

#define FIRST_ROW 1


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
    isLevel2 = false;
    isAlternative = false;
    isLocked = false;
    m_layout = new QGraphicsGridLayout(this);
    extendedKeys = false;
    basicKeys = false;

    tooltip = new Tooltip("");

    switcher = new Plasma::Label(this);

    m_layout->addItem(switcher, 0, 0, 1, 10, Qt::AlignRight);
    m_layout->setRowMinimumHeight ( 0, 10 );
    m_layout->setRowPreferredHeight ( 0, 10 );
    m_layout->setRowMaximumHeight ( 0, 15 );
    QObject::connect(switcher, SIGNAL( linkHovered(QString) ), parent, SLOT( toggleMode() ) );

    engine = dataEngine("keystate");
    if(engine){
	engine -> connectAllSources(this);
    }
}


PlasmaboardWidget::~PlasmaboardWidget(){
    delete switcher;
    delete tooltip;
}

void PlasmaboardWidget::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data){

    if ( sourceName == "Shift" ){
	if ( data["Pressed"].toBool() ){
	    emit shiftKey(true);
	    isLevel2 = true;
	}
	else{
	    emit shiftKey(false);
	    isLevel2 = false;
	}
    }
    
    else if ( sourceName == "Caps Lock" ) {
	if ( data["Pressed"].toBool() )
	    isLocked = true;
	else
	    isLocked = false;
    }

    else if ( sourceName == "AltGr" ) {
	if ( data["Pressed"].toBool() ){
	    isAlternative = true;
	    emit altGrKey(true);
	}
	else{
	    isAlternative = false;
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

void PlasmaboardWidget::resetKeyboard(){

	if( basicKeys ){
		int i = m_layout->count() - 1;
		while(i >= 0){
			m_layout->removeAt(i--);
		}

                qDeleteAll(funcKeys);
                funcKeys.clear();

                qDeleteAll(alphaKeys);
                alphaKeys.clear();

		basicKeys = false;
	}

	if( extendedKeys ) {
                qDeleteAll(extKeys);
                extKeys.clear();
		extendedKeys = false;
	}
}

void PlasmaboardWidget::initExtendedKeyboard(){
	extendedKeys = true;

	int i = 0;
	while ( i < 42 ) {
                extKeys << new FuncKey(this);
		QObject::connect(extKeys[i], SIGNAL( clicked() ), this, SLOT( clear() ) );
		i++;
	}

	delete extKeys[ARROWTOPKEY];
	delete extKeys[ARROWLEFTKEY];
	delete extKeys[ARROWBOTTOMKEY];
	delete extKeys[ARROWRIGHTKEY];
	extKeys[ARROWTOPKEY] = new ArrowTopKey(this);
	extKeys[ARROWLEFTKEY] = new ArrowLeftKey(this);
	extKeys[ARROWBOTTOMKEY] = new ArrowBottomKey(this);
	extKeys[ARROWRIGHTKEY] = new ArrowRightKey(this);

	m_layout->addItem(extKeys[0], FIRST_ROW, 0, 1, 2);
	i = 1;
	for(i = 1; i < 5; i++){
		m_layout->addItem(extKeys[i], FIRST_ROW, (i*2)+1, 1, 2);
	}
	for(i = 5; i < 9; i++){
		m_layout->addItem(extKeys[i], FIRST_ROW, (i*2)+2, 1, 2);
	}
	for(i = 9; i < 13; i++){
		m_layout->addItem(extKeys[i], FIRST_ROW, (i*2)+3, 1, 2);
	}

	for(i = 13; i < 16; i++){
		m_layout->addItem(extKeys[i], FIRST_ROW, (i*2)+4, 1, 2);
	}

	extKeys[0]->setKey(XK_Escape, true, i18nc("The escape key on a keyboard", "Esc"));
        extKeys[1]->setKey(XK_F1, true, i18n("F1"));
        extKeys[2]->setKey(XK_F2, true, i18n("F2"));
        extKeys[3]->setKey(XK_F3, true, i18n("F3"));
        extKeys[4]->setKey(XK_F4, true, i18n("F4"));
        extKeys[5]->setKey(XK_F5, true, i18n("F5"));
        extKeys[6]->setKey(XK_F6, true, i18n("F6"));
        extKeys[7]->setKey(XK_F7, true, i18n("F7"));
        extKeys[8]->setKey(XK_F8, true, i18n("F8"));
        extKeys[9]->setKey(XK_F9, true, i18n("F9"));
        extKeys[10]->setKey(XK_F10, true, i18n("F10"));
        extKeys[11]->setKey(XK_F11, true, i18n("F11"));
        extKeys[12]->setKey(XK_F12, true, i18n("F12"));
	extKeys[13]->setKey(XK_Print, true, i18nc("The print key on a keyboard", "Print"));
	extKeys[14]->setKey(XK_Num_Lock, true, i18nc("The num key on a keyboard", "Num"));
	extKeys[15]->setKey(XK_Pause, true, i18nc("The pause key on a keyboard", "Pause"));
	
	m_layout->addItem(extKeys[ARROWLEFTKEY], FIRST_ROW + 5, 29, 1, 2);

	int col = 31;

	m_layout->addItem(extKeys[HOMEKEY], FIRST_ROW + 1, col, 1, 2);
	extKeys[HOMEKEY]->setKey(XK_Home, true, i18nc("The home key on a keyboard", "Home"));

	m_layout->addItem(extKeys[ENDKEY], FIRST_ROW + 2, col, 1, 2);
	extKeys[ENDKEY]->setKey(XK_End, true, i18nc("The end key on a keyboard", "End"));

	m_layout->addItem(extKeys[INSKEY], FIRST_ROW + 3, col, 1, 2);
	extKeys[INSKEY]->setKey(XK_Insert, true, i18nc("The insert key on a keyboard", "Ins"));

	m_layout->addItem(extKeys[ARROWTOPKEY], FIRST_ROW + 4, col, 1, 2);
	m_layout->addItem(extKeys[ARROWBOTTOMKEY], FIRST_ROW + 5, col, 1, 2);

	col+=2;

	m_layout->addItem(extKeys[ARROWRIGHTKEY], FIRST_ROW + 5, col, 1, 2);
	m_layout->addItem(extKeys[DELKEY], FIRST_ROW + 1, col, 1, 2);
	extKeys[DELKEY]->setKey(XK_Delete, true, i18nc("The delete key on a keyboard", "Del"));
	m_layout->addItem(extKeys[PGUPKEY], FIRST_ROW + 2, col, 1, 2);
	extKeys[PGUPKEY]->setKey(XK_Page_Up, true, i18nc("The page up key on a keyboard", "PgUp"));
	m_layout->addItem(extKeys[PGDOWNKEY], FIRST_ROW + 3, col, 1, 2);
	extKeys[PGDOWNKEY]->setKey(XK_Page_Down, true, i18nc("The page down key on a keyboard", "PgDn"));

	col+=3;

	m_layout->addItem(extKeys[26], FIRST_ROW + 2, col, 1, 2);
        extKeys[26]->setKey(XK_KP_7, true, i18n("7"));
	m_layout->addItem(extKeys[27], FIRST_ROW + 3, col, 1, 2);
        extKeys[27]->setKey(XK_KP_4, true, i18n("4"));
	m_layout->addItem(extKeys[28], FIRST_ROW + 4, col, 1, 2);
        extKeys[28]->setKey(XK_KP_1, true, i18n("1"));
	m_layout->addItem(extKeys[29], FIRST_ROW + 5, col, 1, 4);
        extKeys[29]->setKey(XK_KP_0, true, i18n("0"));

	col+=2;

	m_layout->addItem(extKeys[30], FIRST_ROW + 1, col, 1, 2);
        extKeys[30]->setKey(XK_KP_Divide, true, i18n("/"));
	m_layout->addItem(extKeys[31], FIRST_ROW + 2, col, 1, 2);
        extKeys[31]->setKey(XK_KP_8, true, i18n("8"));
	m_layout->addItem(extKeys[32], FIRST_ROW + 3, col, 1, 2);
        extKeys[32]->setKey(XK_KP_5, true, i18n("5"));
	m_layout->addItem(extKeys[33], FIRST_ROW + 4, col, 1, 2);
        extKeys[33]->setKey(XK_KP_2, true, i18n("2"));

	col+=2;

	m_layout->addItem(extKeys[34], FIRST_ROW + 1, col, 1, 2);
        extKeys[34]->setKey(XK_KP_Multiply, true, i18n("*"));
	m_layout->addItem(extKeys[35], FIRST_ROW + 2, col, 1, 2);
        extKeys[35]->setKey(XK_KP_9, true, i18n("9"));
	m_layout->addItem(extKeys[36], FIRST_ROW + 3, col, 1, 2);
        extKeys[36]->setKey(XK_KP_6, true, i18n("6"));
	m_layout->addItem(extKeys[37], FIRST_ROW + 4, col, 1, 2);
        extKeys[37]->setKey(XK_KP_3, true, i18n("3"));
	m_layout->addItem(extKeys[38], FIRST_ROW + 5, col, 1, 2);
        extKeys[38]->setKey(XK_KP_Separator, true, i18n("."));

	col+=2;

	m_layout->addItem(extKeys[39], FIRST_ROW + 1, col, 1, 2);
        extKeys[39]->setKey(XK_KP_Subtract, true, i18n("-"));
	m_layout->addItem(extKeys[40], FIRST_ROW + 2, col, 2, 2);
        extKeys[40]->setKey(XK_KP_Add, true, i18n("+"));
	m_layout->addItem(extKeys[41], FIRST_ROW + 4, col, 2, 2);
	extKeys[41]->setKey(XK_KP_Enter, true, i18nc("The enter key on a keyboard", "Enter"));

	initBasicKeyboard(1);



	int t_width = size().width() / 30;
	for(int i = 30; i < 44; i++){
		m_layout->setColumnPreferredWidth(i,t_width);
	}
}

void PlasmaboardWidget::initBasicKeyboard(int offset){

	basicKeys = true;
	offset += FIRST_ROW;

	// create objects

        funcKeys.append( new BackspaceKey(this) );
        funcKeys << new TabKey(this);
        funcKeys << new EnterKey(this);
        funcKeys << new CapsKey(this);
        funcKeys << new ShiftKey(this);
        funcKeys << new ShiftKey(this);

        int i = 6;
        while ( i < 14 ) {
                funcKeys << new FuncKey(this);
                i++;
        }

	funcKeys[CTLKEY]->setKey(XK_Control_L, false, i18nc("The Ctrl key on a keyboard", "Ctrl"));
	QObject::connect(this, SIGNAL( controlKey(bool) ), funcKeys[CTLKEY], SLOT( toggle(bool) ) );
	funcKeys[SUPER_L_KEY]->setKey(XK_Super_L, false, i18nc("The meta (windows) key on a keyboard", "Super"));
	QObject::connect(this, SIGNAL( superKey(bool) ), funcKeys[SUPER_L_KEY], SLOT( toggle(bool) ) );
	funcKeys[ALT_L_KEY]->setKey(XK_Alt_L, false, i18nc("The alt key on a keyboard", "Alt"));
	QObject::connect(this, SIGNAL( altKey(bool) ), funcKeys[ALT_L_KEY], SLOT( toggle(bool) ) );
        funcKeys[SPACE]->setKeycode(XK_space, true);
	funcKeys[ALTGRKEY]->setKey(XK_ISO_Level3_Shift, false, i18nc("The Alt Gr key on a keyboard", "Alt Gr"));
	QObject::connect(this, SIGNAL( altGrKey(bool) ), funcKeys[ALTGRKEY], SLOT( toggle(bool) ) );
	funcKeys[SUPER_R_KEY]->setKey(XK_Super_L, false, i18n("Super"));
	QObject::connect(this, SIGNAL( superKey(bool) ), funcKeys[SUPER_R_KEY], SLOT( toggle(bool) ) );
	funcKeys[MENU]->setKey(XK_Menu, true, i18nc("The menu key on a keyboard", "Menu"));
	QObject::connect(this, SIGNAL( menuKey(bool) ), funcKeys[MENU], SLOT( toggle(bool) ) );
	funcKeys[CONTROL_LEFT]->setKey(XK_Control_L, false, i18nc("The Ctrl key on a keyboard", "Ctrl"));
	QObject::connect(this, SIGNAL( controlKey(bool) ), funcKeys[CONTROL_LEFT], SLOT( toggle(bool) ) );

        // set Keymap

        alphaKeys << new AlphaNumKey(this, 49);

        for ( i = 10; i < 22; i++ ){
               alphaKeys << new AlphaNumKey(this, i);
        }

        for ( i = 24; i < 36; i++ ){
               alphaKeys << new AlphaNumKey(this, i);
        }

        for ( i = 38; i < 49; i++ ){
               alphaKeys << new AlphaNumKey(this, i);
        }

        alphaKeys << new AlphaNumKey(this, 51);
        alphaKeys << new AlphaNumKey(this, 94);

        for ( i = 52; i < 62; i++ ){
               alphaKeys << new AlphaNumKey(this, i);
        }


	// create layout

	int numberOfCols = 30;

	int x;
	int row = offset;
	for(x = 0; x<=12; x++){
		m_layout->addItem(alphaKeys[x], row, x*2, 1, 2);
	}
	m_layout->addItem(funcKeys[BACKSPACEKEY], row, x*2, 1, 4);

	row++;

	m_layout->addItem(funcKeys[TABKEY], row, 0, 1, 3);
	int t = 3;
	for(x = 13; x<=24; x++){
		m_layout->addItem(alphaKeys[x], row, t, 1, 2);
		t+=2;
	}
	m_layout->addItem(funcKeys[ENTERKEY], row, t+1, 2, 2);

	row++;

	m_layout->addItem(funcKeys[CAPSKEY], row, 0, 1, 4);
	t = 4;
	for(x = 25; x<=36; x++){
		m_layout->addItem(alphaKeys[x], row, t, 1, 2);
		t+=2;
	}

	row++;

	m_layout->addItem(funcKeys[SHIFT_L_KEY], row, 0, 1, 3);
	t = 3;
	for(x = 37; x<=47; x++){
		m_layout->addItem(alphaKeys[x], row, t, 1, 2);
		t+=2;
	}
	m_layout->addItem(funcKeys[SHIFT_R_KEY], row, t, 1, 5);

	row++;

	m_layout->addItem(funcKeys[CTLKEY], row, 0, 1, 2);
	m_layout->addItem(funcKeys[SUPER_L_KEY], row, 2, 1, 2);
	m_layout->addItem(funcKeys[ALT_L_KEY], row, 4, 1, 2);
	m_layout->addItem(funcKeys[SPACE], row, 6, 1, 14);
	m_layout->addItem(funcKeys[ALTGRKEY], row, 20, 1, 2);
	m_layout->addItem(funcKeys[SUPER_R_KEY], row, 22, 1, 2);
	m_layout->addItem(funcKeys[MENU], row, 24, 1, 2);
	m_layout->addItem(funcKeys[CONTROL_LEFT], row, 26, 1, 3);

	for(int i = 0; i<numberOfCols; i++){
                m_layout->setColumnPreferredWidth(i,size().width()/numberOfCols);
	}

	for(int i = 30; i < 44; i++){
		m_layout->setColumnPreferredWidth(i, 0);
	}

	m_layout->setSpacing(0);
	m_layout->setContentsMargins(0,0,0,0);


}

void PlasmaboardWidget::paint(QPainter *p,
                const QStyleOptionGraphicsItem *option,
                 QWidget*)
{
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);
}

void PlasmaboardWidget::relabelKeys() {
	foreach (AlphaNumKey* key, alphaKeys){
	    key->switchKey(isLevel2, isAlternative, isLocked);
	}
}

void PlasmaboardWidget::clearAnything(){
	if ( isLocked ) {
		funcKeys[CAPSKEY]->sendKeycodePress();
		funcKeys[CAPSKEY]->sendKeycodeRelease();
	}
	clear();
}

void PlasmaboardWidget::clear(){

	bool change = false;
	if( funcKeys[SHIFT_L_KEY]->toggled() || funcKeys[SHIFT_R_KEY]->toggled() ){
		Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Shift_L));
		funcKeys[SHIFT_L_KEY]->toggleOff();
		funcKeys[SHIFT_R_KEY]->toggleOff();
		isLevel2 = false;
		change = true;
	}
	if( funcKeys[ALTGRKEY]->toggled() ){
		Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_ISO_Level3_Shift));
		funcKeys[ALTGRKEY]->toggleOff();
		isAlternative = false;
		change = true;
	}

	if(change){
		relabelKeys();
	}

	Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Control_L));
	funcKeys[CTLKEY]->toggleOff();
        funcKeys[CONTROL_LEFT]->toggleOff();
	Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Meta_L));
	funcKeys[SUPER_L_KEY]->toggleOff();
	funcKeys[SUPER_R_KEY]->toggleOff();
	Helpers::fakeKeyRelease(Helpers::keysymToKeycode(XK_Alt_L));
	funcKeys[ALT_L_KEY]->toggleOff();

	clearTooltip();
}


void PlasmaboardWidget::setTooltip(QString text, QSizeF buttonSize, QPointF position){
    tooltip -> setText( text );
    tooltip -> move( popupPosition(buttonSize.toSize()) + QPoint(position.x() - buttonSize.width() / 2 , position.y() - buttonSize.height()) );
    tooltip -> resize( (buttonSize * 2).toSize() );
    tooltip -> show();

    //tooltipTimer->start(300);
}

void PlasmaboardWidget::clearTooltip(){

    tooltip->hide();
}

