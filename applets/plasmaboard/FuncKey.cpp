/***************************************************************************
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


#include "FuncKey.h"

#include <kpushbutton.h>
#include <QPainter>
#include <plasma/theme.h>

FuncKey::FuncKey(const QPoint &relativePosition, const QSize &relativeSize, unsigned int keycode, const QString &label)
    : AlphaNumKey(relativePosition, relativeSize, keycode)
{
    setLabel(label);
}

bool FuncKey::repeats() const
{
    return false;
}

void FuncKey::paintArrow(QPainter *painter)
{
    int unit = qMin(size().width(), size().height()) / 8;
    painter->drawLine(-1 * unit, 0 , 3 * unit, 0);

    const QPointF points[3] = {
        QPointF(-3 * unit, 0),
        QPointF(-1 * unit, 1 * unit),
        QPointF(-1 * unit, -1 * unit),
    };

    painter->drawConvexPolygon(points, 3);
}

void FuncKey::paintLabel(QPainter *painter)
{
    painter->save();
    int fontSize = qMin(size().width(), size().height()) / 5 - (label().size() / 6 + 1);
    painter->setFont(QFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).toString(), fontSize));
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    painter->drawText(rect(), Qt::AlignCenter, label());
    painter->restore();
}
