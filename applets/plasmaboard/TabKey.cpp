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


#include "TabKey.h"
#include <QPainter>
#include <plasma/theme.h>

TabKey::TabKey(PlasmaboardWidget *parent) : FuncKey::FuncKey(parent){
	setKeycode(XK_Tab, true);
}

void TabKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	Plasma::PushButton::paint(painter, option, widget);

	setUpPainter(painter);
	painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));

	painter->drawLine(-1, -2 , 3, -2);
	QPointF points[3] = {
	     QPointF(-3, -2),
	     QPointF(-1, -1),
	     QPointF(-1, -3),
	 };
	painter->drawConvexPolygon(points, 3);
	painter->drawLine(-3, -1 , -3, -3);

	painter->drawLine(-3, 0 , 1, 0);
	points = {
	     QPointF(1, -1),
	     QPointF(3, 0),
	     QPointF(1, 1),
	 };
	painter->drawConvexPolygon(points, 3);
	painter->drawLine(3, 1 , 3, -1);

}
