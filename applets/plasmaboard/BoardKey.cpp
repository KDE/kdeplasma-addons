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

#include <QPainter>
#include <QString>
#include <plasma/theme.h>
#include "BoardKey.h"
#include "widget.h"

BoardKey::BoardKey(PlasmaboardWidget *parent):
	Plasma::PushButton(parent),fontSize(6) {
	//this->setOrientation(Qt::Horizontal);
	//this->setDrawBackground(true);
	setMinimumSize(10,10);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored, QSizePolicy::DefaultType);
	setFocusPolicy(Qt::NoFocus);
}

BoardKey::~BoardKey() {
	// TODO Auto-generated destructor stub
}

void BoardKey::setText(QString text) {
        labelText = text;
	fontSize = (text.size() > 1) ? ((text.size() > 3) ? 2 : 4) : 6;
        //Plasma::PushButton::setText(text);
}

QString BoardKey::text() {
        return labelText;
}

unsigned int BoardKey::getKeycode() {
	return keycode;
}

void BoardKey::sendKeycodePress() {}
void BoardKey::sendKeycodeRelease() {}
void BoardKey::sendKeycodeToggled() {}

void BoardKey::setUpPainter(QPainter *painter){
	painter->setRenderHints(QPainter::Antialiasing);
	painter->setPen(QPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor)));

	painter->translate(contentsRect().center());
	double mul = qMin(contentsRect().width(), contentsRect().height()) / 10;
	painter->scale(mul, mul);

}

void BoardKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        Plasma::PushButton::paint(painter, option, widget);

	setUpPainter(painter); // scales the matrix
	painter->scale(0.1, 0.1);


	painter->setFont(QFont ( Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).toString(), fontSize*10));
	painter->drawText(QRect(-50,-50,100,100), Qt::AlignCenter , labelText); // don't know why rect must be like that
}
