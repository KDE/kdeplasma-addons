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


#include "CapsKey.h"
#include <QPainter>
#include <plasma/theme.h>

CapsKey::CapsKey(PlasmaboardWidget *parent) : FuncKey::FuncKey(parent){
	setKeycode(XK_Caps_Lock, true);
}

void CapsKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	Plasma::PushButton::paint(painter, option, widget);

	setUpPainter(painter);

	QRectF rect = contentsRect();
	int height = rect.height();
	QPointF center = rect.center();

	double unitHeight = height / 4;
	double unitWidth = unitHeight / 2;

	const QPointF points[7] = {
		     QPointF(center.x(), center.y() + unitHeight),
		     QPointF(center.x() + 2 * unitWidth, center.y()),
		     QPointF(center.x() + unitWidth, center.y()),
		     QPointF(center.x() + unitWidth, center.y() - unitHeight),
		     QPointF(center.x() - unitWidth, center.y() - unitHeight),
		     QPointF(center.x() - unitWidth, center.y()),
		     QPointF(center.x() - 2 * unitWidth, center.y())
	};

	painter->drawConvexPolygon(points, 7);

}
