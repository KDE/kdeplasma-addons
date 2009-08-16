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

	QRectF rect = contentsRect();
	int width = rect.width();
	int height = rect.height();

	double unitWidth = width / 4;
	double unitHeight = height / 4;


	painter->drawLine(rect.left() + 1.5 * unitWidth,
			rect.top() + unitHeight,
			rect.right() - unitWidth,
			rect.top() + unitHeight);
	const QPointF points[3] = {
	     QPointF(rect.left() + unitWidth, rect.top() + unitHeight),
	     QPointF(rect.left() + 1.5 * unitWidth, rect.top() + unitHeight/2),
	     QPointF(rect.left() + 1.5 * unitWidth, rect.top() + 1.5*unitHeight)
	 };
	painter->drawConvexPolygon(points, 3);
	painter->drawLine(rect.left() + unitWidth - 1,
				rect.top() + unitHeight / 2,
				rect.left() + unitWidth - 1,
				rect.top() + 1.5*unitHeight);

	painter->drawLine(rect.right() - 1.5 * unitWidth,
			rect.bottom() - 2 * unitHeight,
			rect.left() + unitWidth,
			rect.bottom() - 2 * unitHeight);
	const QPointF pointsTwo[3] = {
	     QPointF(rect.right() - unitWidth, rect.bottom() - 2 * unitHeight),
	     QPointF(rect.right() - 1.5 * unitWidth, rect.bottom() - 1.5 * unitHeight),
	     QPointF(rect.right() - 1.5 * unitWidth, rect.bottom() - 2.5 * unitHeight)
	 };
	painter->drawLine(rect.right() - unitWidth + 1,
				rect.bottom() - 1.5 * unitHeight,
				rect.right() - unitWidth + 1,
				rect.bottom() - 2.5 * unitHeight);

	painter->drawConvexPolygon(pointsTwo, 3);
}
