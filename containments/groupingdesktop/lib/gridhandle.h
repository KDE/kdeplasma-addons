/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#ifndef GRIDHANDLE_H
#define GRIDHANDLE_H

#include "handle.h"

class GridHandle : public Handle
{
    Q_OBJECT
    public:
        GridHandle(GroupingContainment *containment, Plasma::Applet *applet);
        GridHandle(GroupingContainment *containment, AbstractGroup *group);
        virtual ~GridHandle();

        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    public slots:
        void widgetResized();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    private:
        bool m_moving;
        QPointF m_startPos;
        int m_savedZValue;
};

#endif
