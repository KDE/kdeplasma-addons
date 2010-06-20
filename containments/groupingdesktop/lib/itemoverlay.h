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

#ifndef ITEMOVERLAY_H
#define ITEMOVERLAY_H

#include <QtGui/QGraphicsWidget>

class QGraphicsLinearLayout;

class ItemOverlay : public QGraphicsWidget
{
    Q_OBJECT
    public:
        explicit ItemOverlay(QGraphicsWidget *item, Qt::WindowFlags wFlags = 0);
        ~ItemOverlay();

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
        QGraphicsWidget *item() const;
        bool isMoving() const;
        void setZ(int value);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    signals:
        void startMoving();
        void movedOf(qreal x, qreal y, const QPointF &pos);
        void endMoving();
        void itemMovedOutside(qreal x, qreal y);

    private slots:
        void syncGeometry();
        void delayedSyncGeometry();

    private:
        QGraphicsWidget *m_item;
        QGraphicsLinearLayout *m_layout;
        bool m_moving;
        QPointF m_startPos;
        int m_savedZValue;
};

#endif // APPLETOVERLAY_H
