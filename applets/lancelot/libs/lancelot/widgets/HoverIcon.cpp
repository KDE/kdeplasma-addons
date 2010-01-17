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

#include "HoverIcon.h"

#include <QBasicTimer>

#define ACTIVATION_TIME 300

namespace Lancelot
{

class HoverIcon::Private {
public:
    Private(HoverIcon * parent)
      : q(parent),
        activationMethod(ClickActivate)
    {
    }

    void startTimer()
    {
        timer.start(ACTIVATION_TIME, q);
    }

    void stopTimer()
    {
        timer.stop();
    }


    HoverIcon * q;
    int activationMethod;
    static QBasicTimer timer;
};

QBasicTimer HoverIcon::Private::timer = QBasicTimer();

HoverIcon::HoverIcon(QGraphicsItem * parent)
  : Plasma::IconWidget(parent),
    d(new Private(this))
{

}

HoverIcon::HoverIcon(const QString & text, QGraphicsItem * parent)
  : Plasma::IconWidget(text, parent),
    d(new Private(this))
{

}

HoverIcon::HoverIcon(const QIcon & icon, const QString & text, QGraphicsItem * parent)
  : Plasma::IconWidget(icon, text, parent),
    d(new Private(this))
{

}

HoverIcon::~HoverIcon()
{
    delete d;
}

void HoverIcon::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->timer.timerId()) {
        d->stopTimer();
        if (d->activationMethod == HoverActivate) {
            emit activated();
        }
    }
    Plasma::IconWidget::timerEvent(event);
}

void HoverIcon::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    if (d->activationMethod == HoverActivate) {
        d->startTimer();
    }
    Plasma::IconWidget::hoverEnterEvent(event);
}

void HoverIcon::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    d->stopTimer();
    Plasma::IconWidget::hoverLeaveEvent(event);
}

void HoverIcon::setActivationMethod(int method)
{
    if (method == ExtenderActivate) {
        method = ClickActivate;
    }
    if (d->activationMethod == method)
        return;
    d->activationMethod = method;
}

int HoverIcon::activationMethod() const
{
    return d->activationMethod;
}

} // namespace Lancelot

#include "HoverIcon.moc"

