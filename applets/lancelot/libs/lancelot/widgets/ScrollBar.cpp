/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "ScrollBar.h"
#include <KDebug>

#include "ExtenderButton.h"
#include <QTimer>
#include <QGraphicsSceneMouseEvent>

namespace Lancelot
{

class ScrollBar::Private {
public:
};

ScrollBar::ScrollBar(QGraphicsWidget * parent)
  : Plasma::ScrollBar(parent), d(new Private())
{
    setPageStep(100);
    setSingleStep(20);
}

ScrollBar::~ScrollBar()
{
    delete d;
}

void ScrollBar::setMinimum(int min)
{
    setRange(min, maximum());
}

void ScrollBar::setMaximum(int max)
{
    setRange(minimum(), max);
}

void ScrollBar::setViewSize(int size)
{
    setPageStep(size);
}

void ScrollBar::setPageSize(int size)
{
    setPageStep(size);
}

void ScrollBar::setStepSize(int size)
{
    setPageStep(size);
}

void ScrollBar::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    Plasma::ScrollBar::wheelEvent(event);
}

} // namespace Lancelot

