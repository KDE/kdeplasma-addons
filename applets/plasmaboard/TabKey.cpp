/*/*************************************************************************
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


#include "TabKey.h"
#include "Helpers.h"
#include <QPainter>


TabKey::TabKey(QPoint relativePosition, QSize relativeSize) :
        FuncKey(relativePosition, relativeSize, Helpers::keysymToKeycode(XK_Tab), QString()){

}

void TabKey::paint(QPainter *painter){

    FuncKey::paint(painter);

    painter->save();
    setUpPainter(painter);
    painter->scale(size().height()/10.0, size().height()/10.0);

	painter->drawLine(-1, -2 , 3, -2);
	QPointF points[3] = {
	     QPointF(-3, -2),
	     QPointF(-1, -1),
	     QPointF(-1, -3),
	 };
	painter->drawConvexPolygon(points, 3);
	painter->drawLine(-3, -1 , -3, -3);

	painter->drawLine(-3, 0 , 1, 0);
	QPointF points2[3] = {
	     QPointF(1, -1),
	     QPointF(3, 0),
	     QPointF(1, 1),
	 };
	painter->drawConvexPolygon(points2, 3);
	painter->drawLine(3, 1 , 3, -1);

    painter->restore();
}
