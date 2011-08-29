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
#include "Helpers.h"
#include <QPainter>

#include <Plasma/Theme>

CapsKey::CapsKey(QPoint relativePosition, QSize relativeSize) :
    StickyKey(relativePosition, relativeSize, Helpers::keysymToKeycode(XK_Caps_Lock), QString())
{

}

void CapsKey::paint(QPainter *painter)
{
    FuncKey::paint(painter);

    painter->save();
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    painter->setRenderHint(QPainter::Antialiasing);
    setUpPainter(painter);

    int unit = qMin(size().width(), size().height()) / 8;

    const QPointF points[7] = {
        QPointF(0, 3 * unit),
        QPointF(-2 * unit, 0),
        QPointF(-1 * unit, 0),
        QPointF(-1 * unit, -2 * unit),
        QPointF(1 * unit, -2 * unit),
        QPointF(1 * unit, 0),
        QPointF(2 * unit, 0)
    };

    painter->drawConvexPolygon(points, 7);
    painter->restore();
}
