/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LIBLANCELOT_H
#define LIBLANCELOT_H

#include <QMetaObject>
#include <QWidget>

#include <limits>

namespace Lancelot
{

enum ExtenderPosition {
    NoExtender = 0,
    RightExtender = 1,
    LeftExtender = 2,
    TopExtender = 3,
    BottomExtender = 4
};

enum ActivationMethod {
    HoverActivate = 0,
    ClickActivate = 1,
    ExtenderActivate = 2
};

#define MAX_WIDGET_SIZE QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)

#define QREAL_INFINITY std::numeric_limits<qreal>::infinity()

#define L_WIDGET

} // namespace Lancelot

#endif // LIBLANCELOT_H

