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

#ifndef TIMERDIGIT_H
#define TIMERDIGIT_H

#include <Plasma/SvgWidget>

class QGraphicsItem;
class QGraphicsSceneWheelEvent;

namespace Plasma
{
    class Svg;
}

class TimerDigit : public Plasma::SvgWidget
{
    Q_OBJECT

    public:
        TimerDigit(Plasma::Svg *svg, int seconds, QGraphicsItem *parent = 0);
        void wheelEvent(QGraphicsSceneWheelEvent * event);

        virtual ~TimerDigit();

    Q_SIGNALS:
        void changed(int value);

    private:
        int m_seconds;
};

#endif
