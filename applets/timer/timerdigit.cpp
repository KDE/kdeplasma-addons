/*
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>
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

#include "timerdigit.h"

#include <QGraphicsSceneMouseEvent>

TimerDigit::TimerDigit(Plasma::Svg *svg, int seconds, QGraphicsItem *parent)
    : SvgWidget(svg, "0", parent)
{
    m_seconds = seconds;
}

TimerDigit::~TimerDigit()
{

}

void TimerDigit::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    emit changed((event->delta() < 0) ? -m_seconds : m_seconds);
}

#include "timerdigit.moc"
