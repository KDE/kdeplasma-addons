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


#include "ShiftKey.h"
#include <QPainter>
#include <plasma/theme.h>

ShiftKey::ShiftKey(PlasmaboardWidget *parent) : FuncKey::FuncKey(parent){
	setKeycode(XK_Shift_L, false);
}

void ShiftKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	Plasma::PushButton::paint(painter, option, widget);

	setUpPainter(painter);


	const QPointF points[7] = {
		     QPointF(0, -3),
		     QPointF(-2, 0),
		     QPointF(-1, 0),
		     QPointF(-1, 2),
		     QPointF( 1, 2),
		     QPointF( 1, 0),
		     QPointF( 2, 0)
	};

	painter->drawConvexPolygon(points, 7);

}
