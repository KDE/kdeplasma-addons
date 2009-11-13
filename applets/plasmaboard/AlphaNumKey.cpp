/***************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>         *
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



#include <kpushbutton.h>
#include "AlphaNumKey.h"
#include "Helpers.h"

AlphaNumKey::AlphaNumKey(PlasmaboardWidget *parent, unsigned int keysym):
        BoardKey::BoardKey(parent){


        QObject::connect(static_cast<const KPushButton*>(this->nativeWidget()), SIGNAL( pressed() ), this, SLOT( sendKeycodePress() ) );
        QObject::connect(static_cast<const KPushButton*>(this->nativeWidget()), SIGNAL( released() ), this, SLOT( sendKeycodeRelease() ) );
        QObject::connect(this, SIGNAL( clicked() ), parent, SLOT( clear() ) );
	QObject::connect(this, SIGNAL( keySend ( QString, QSizeF, QPointF ) ), parent, SLOT( setTooltip( QString, QSizeF, QPointF ) ) );
        setKeycode(keysym);
}

AlphaNumKey::~AlphaNumKey() {

}

void AlphaNumKey::setKeycode(unsigned int keycodeP) {
	keycode = keycodeP;
	setLabel(0);
}

void AlphaNumKey::setLabel(int level){
	setText( Helpers::mapToUnicode(Helpers::keycodeToKeysym(getKeycode(),level)) );
}

void AlphaNumKey::switchKey(bool isLevel2, bool isAlternative, bool isLocked){
	if(isLocked){
		isLevel2 = !isLevel2;
	}
	if(isLevel2){
		isAlternative ? setLabel(4) : setLabel(1);
	}
	else{
		setLabel(0);
		/*isAlternative ?
		setText(QChar(Helpers::mapToUnicode(Helpers::keycodeToKeysym(getKeycode(),0)))) :
		setText(QChar(Helpers::mapToUnicode(Helpers::keycodeToKeysym(getKeycode(),0))));*/
        }
	update();
}

void AlphaNumKey::sendKeycodePress() {
	emit keySend(text(), size(), pos());
        Helpers::fakeKeyPress(getKeycode());
}

void AlphaNumKey::sendKeycodeRelease() {
	Helpers::fakeKeyRelease(getKeycode());
}


