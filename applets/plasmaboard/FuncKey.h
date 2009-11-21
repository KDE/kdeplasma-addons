/****************************************************************************
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


#ifndef FUNCKEY_H_
#define FUNCKEY_H_

#include "BoardKey.h"

class PlasmaboardWidget;

class FuncKey: public BoardKey {

    Q_OBJECT

public:
	FuncKey(PlasmaboardWidget *parent);
	void toggleOn();
	void toggleOff();
	bool toggled();
	void setKeycode(unsigned int code, bool sendUp);
	void setKey(unsigned int code, bool sendUp, const QString text);


public Q_SLOTS:
	virtual void sendKeycodePress();
	virtual void sendKeycodeRelease();
	virtual void sendKeycodeToggled();
	void toggle(bool toggle);


private:
	QString oldStyle;
	bool pressed;

protected:
	void paintArrow(QPainter *painter);
};

#endif /* FUNCKEY_H_ */
