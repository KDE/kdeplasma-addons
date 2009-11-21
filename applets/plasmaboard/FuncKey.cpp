/***************************************************************************
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


//#include "FuncKey.h"

#include <kpushbutton.h>
#include "FuncKey.h"
#include <QPainter>
#include <plasma/theme.h>
#include "Helpers.h"

FuncKey::FuncKey(PlasmaboardWidget *parent):
	BoardKey::BoardKey(parent), pressed(false) {
}

void FuncKey::toggleOn(){
	nativeWidget()->setDown(true);
	pressed = true;
}

void FuncKey::toggleOff(){
	nativeWidget()->setDown(false);
	pressed = false;
}

void FuncKey::toggle(bool toggle){
	nativeWidget()->setDown(toggle);
	pressed = toggle;
}

bool FuncKey::toggled(){
	return pressed;
}

void FuncKey::setKeycode(unsigned int code, bool sendUp){
	keycode = Helpers::keysymToKeycode(code);
	if(sendUp){
		QObject::connect(static_cast<const KPushButton*>(this->nativeWidget()), SIGNAL( pressed() ), this, SLOT( sendKeycodePress() ) );
                QObject::connect(static_cast<const KPushButton*>(this->nativeWidget()), SIGNAL( released() ), this, SLOT( sendKeycodeRelease() ) );
	}
	else {
		QObject::connect(this, SIGNAL( clicked() ), this, SLOT( sendKeycodeToggled() ) );
	}
}


void FuncKey::setKey(unsigned int code, bool sendUp, const QString text) {
        setKeycode(code, sendUp);
        setText(text);
}

void FuncKey::sendKeycodePress() {
	Helpers::fakeKeyPress(getKeycode());
	toggleOn();
}

void FuncKey::sendKeycodeToggled() {
	if( pressed ) {
		sendKeycodeRelease();
		toggleOff();
	}
	else {
		Helpers::fakeKeyPress(getKeycode());
		toggleOn();
	}
}

void FuncKey::sendKeycodeRelease() {
	Helpers::fakeKeyRelease(getKeycode());
	toggleOff();
}

void FuncKey::paintArrow(QPainter *painter){

    	painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
	painter->drawLine(-1, 0 , 3, 0);

	const QPointF points[3] = {
	     QPointF(-3, 0),
	     QPointF(-1, 1),
	     QPointF(-1, -1),
	 };

	painter->drawConvexPolygon(points, 3);
}
