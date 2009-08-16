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


#include "BackspaceKey.h"
#include <QPainter>
#include <plasma/theme.h>

BackspaceKey::BackspaceKey(PlasmaboardWidget *parent) : FuncKey::FuncKey(parent){
	setKeycode(XK_BackSpace, true);
}

void BackspaceKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

	Plasma::PushButton::paint(painter, option, widget);
	setUpPainter(painter);

	QRectF rect = contentsRect();
	int width = rect.width();
	int height = rect.height();
	QPointF center = rect.center();

	painter->drawLine(rect.left() + width / 6 + width / 4, center.y(), rect.right() - width / 4, center.y());

	const QPointF points[3] = {
	     QPointF(rect.left() + width / 4, center.y()),
	     QPointF(rect.left() + width / 6 + width / 4, center.y() - height / 10),
	     QPointF(rect.left() + width / 6 + width / 4, center.y() + height / 10),
	 };

	painter->drawConvexPolygon(points, 3);

}
