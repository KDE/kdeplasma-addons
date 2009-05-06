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
#include "ExtenderButton.h"
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QBasicTimer>

#define SINGLE_SHOT_INTERVAL 50
#define SINGLE_SHOT_MOVE 10

namespace Lancelot
{

class ScrollBar::Private {
public:
    Private()
        : direction(0), hoverScroll(true)
    {}

    QBasicTimer timer;
    int direction;
    bool hoverScroll: 1;
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

void ScrollBar::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    if (isVisible()) {
        Plasma::ScrollBar::wheelEvent(event);
    }
}

void ScrollBar::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    d->timer.stop();
    Plasma::ScrollBar::hoverLeaveEvent(event);
}

void ScrollBar::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
    Plasma::ScrollBar::hoverMoveEvent(event);
    if (!d->hoverScroll) {
        return;
    }

    QPointF pos = event->pos();

    QScrollBar * sbar = qobject_cast
        < QScrollBar * > (widget());

    if (!sbar) {
        return;
    }

    if (sbar->orientation() == Qt::Vertical) {
        if (pos.y() < size().width()) {
            d->direction = -1;
        } else if (pos.y() > size().height() - size().width()) {
            d->direction = +1;
        } else {
            d->direction = 0;
        }
    } else {
        if (pos.x() < size().height()) {
            d->direction = -1;
        } else if (pos.x() > size().width() - size().height()) {
            d->direction = +1;
        } else {
            d->direction = 0;
        }
    }

    if ((!d->timer.isActive()) && (d->direction != 0)) {
        d->timer.start(SINGLE_SHOT_INTERVAL, this);
    }
}

void ScrollBar::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->timer.timerId()) {
        setValue(value() + d->direction * SINGLE_SHOT_MOVE);
    }
    Plasma::ScrollBar::timerEvent(event);
}

void ScrollBar::setActivationMethod(Lancelot::ActivationMethod method)
{
    d->hoverScroll = (method != Lancelot::ClickActivate);
}

Lancelot::ActivationMethod ScrollBar::activationMethod() const
{
    if (d->hoverScroll) {
        return Lancelot::HoverActivate;
    } else {
        return Lancelot::ClickActivate;
    }
}


} // namespace Lancelot

