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


#ifndef FUNCKEY_H
#define FUNCKEY_H

#include "AlphaNumKey.h"

class PlasmaboardWidget;

class FuncKey: public AlphaNumKey {


public:
    FuncKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString label);
	void toggleOn();
	void toggleOff();
	bool toggled();
	void setKey(unsigned int code, bool sendUp, const QString text);
	void sendKeycodeToggled();
	void toggle(bool toggle);

protected:
    virtual void paint(QPainter *painter);
	virtual void released();

private:
    QString m_label;
    bool is_pressed;
	bool toggler;


protected:
	void paintArrow(QPainter *painter);
};

#endif /* FUNCKEY_H */
