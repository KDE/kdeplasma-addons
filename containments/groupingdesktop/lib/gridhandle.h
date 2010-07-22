/*
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

        void init();
        void detachWidget();
        QRectF boundingRect() const;
        void setHoverPos(const QPointF &hoverPos);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    public slots:
        void widgetResized();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        enum Location {
            NoLocation,
            Top,
            Right,
            Bottom,
            Left
        };

        bool enoughRoom();
        Handle::ButtonType mapToButton(const QPointF &pos);
        QRectF fullRect() const;
        bool isHorizontal() const;

        bool m_moving;
        QPointF m_startPos;
        int m_savedZValue;
        QPointF m_widgetPos;
        QSizeF m_widgetSize;
        Plasma::Svg *m_configureIcons;
        Handle::ButtonType m_lastButton;
        Location m_location;
        const int SIZE;
};

#endif
