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


#include "ArrowTopKey.h"
#include "Helpers.h"
#include <QPainter>

#include <Plasma/Theme>

ArrowTopKey::ArrowTopKey(QPoint relativePosition, QSize relativeSize) :
    FuncKey(relativePosition, relativeSize, Helpers::keysymToKeycode(XK_Up), QString())
{

}

void ArrowTopKey::paint(QPainter *painter)
{
    FuncKey::paint(painter);

    painter->save();
    setUpPainter(painter);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    painter->rotate(90);
    paintArrow(painter);
    painter->restore();
}
