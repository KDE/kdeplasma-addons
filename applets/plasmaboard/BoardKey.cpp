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
	Plasma::PushButton(parent){
	//this->setOrientation(Qt::Horizontal);
	//this->setDrawBackground(true);
	setMinimumSize(10,10);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored, QSizePolicy::DefaultType);
}

BoardKey::~BoardKey() {
	// TODO Auto-generated destructor stub
}

void BoardKey::setText(QString text) {
        labelText = text;
	fontDivider = (text.size() > 1) ? ((text.size() > 3) ? 4 : 3) : 2;
        update();
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
	//painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
}

void BoardKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        Plasma::PushButton::paint(painter, option, widget);

        setUpPainter(painter);
	painter->setFont(QFont ( Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).toString(), (qMin(size().height(), size().width()) - 5) / fontDivider ));
        painter->drawText(rect(), Qt::AlignCenter , labelText);
}
