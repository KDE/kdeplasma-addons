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

#include <Plasma/Theme>

TabKey::TabKey(QPoint relativePosition, QSize relativeSize) :
    FuncKey(relativePosition, relativeSize, Helpers::keysymToKeycode(XK_Tab), QString())
{
}

void TabKey::paint(QPainter *painter)
{
    FuncKey::paint(painter);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    setUpPainter(painter);
    int unit = qMin(size().width(), size().height()) / 8;

    painter->drawLine(-1 * unit, -2 * unit , 3 * unit, -2 * unit);
    QPointF points[3] = {
        QPointF(-3 * unit, -2 * unit),
        QPointF(-1 * unit, -1 * unit),
        QPointF(-1 * unit, -3 * unit),
    };
    painter->drawConvexPolygon(points, 3);
    painter->drawLine(-3 * unit, -1 * unit , -3 * unit, -3 * unit);

    painter->drawLine(-3 * unit, 0 , 1 * unit, 0);
    QPointF points2[3] = {
        QPointF(1 * unit, -1 * unit),
        QPointF(3 * unit, 0),
        QPointF(1 * unit, 1 * unit),
    };
    painter->drawConvexPolygon(points2, 3);
    painter->drawLine(3 * unit, 1 * unit , 3 * unit, -1 * unit);

    painter->restore();
}
