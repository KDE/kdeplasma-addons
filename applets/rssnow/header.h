/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef HEADER_HEADER
#define HEADER_HEADER

#include <QGraphicsWidget>
#include <plasma/plasma_export.h>

namespace Plasma
{
    class Svg;
} // namespace Plasma


class Header : public QGraphicsWidget
{
    Q_OBJECT

public:
    Header(QGraphicsItem *parent = 0);


protected:
    virtual void paint(QPainter *p,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget = 0);
    Plasma::Svg *m_icon;
};

#endif
