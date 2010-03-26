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


#include "EnterKey.h"
#include <QPainter>
#include <plasma/theme.h>

EnterKey::EnterKey(QPoint relativePosition, QSize relativeSize) :
        FuncKey(relativePosition, relativeSize, XK_Return, QString()) {

}


void EnterKey::paint(QPainter *painter){
    FuncKey::paint(painter);

	painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));

	painter->drawLine(-1, 0, 3, 0);
	painter->drawLine(3, 0, 3, -1);

	const QPointF points[3] = {
	     QPointF(-3, 0),
	     QPointF(-1, 1),
	     QPointF(-1, -1),
	 };

	painter->drawConvexPolygon(points, 3);

}
