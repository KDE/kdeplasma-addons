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

#ifndef KEY_H_
#define KEY_H_

#include <widget.h>
#include <plasma/widgets/pushbutton.h>

class PlasmaboardWidget;

class BoardKey : public Plasma::PushButton  {
	 Q_OBJECT
public:
	BoardKey(PlasmaboardWidget *parent);
	virtual ~BoardKey();

	unsigned int getKeycode();

	/*
	  Replaces text on the button. Warning! This does not trigger a repaint for performance reasons.
	  If you are updating an already painted button, call update() !
	  */
	void setText(QString text);
        QString text();
private:
	QString labelText;
	int fontSize;

public Q_SLOTS:
	virtual void sendKeycodePress();
	virtual void sendKeycodeRelease();
	virtual void sendKeycodeToggled();

protected:
        void setUpPainter(QPainter *painter);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	unsigned int keycode;

};

#endif /* KEY_H_ */
