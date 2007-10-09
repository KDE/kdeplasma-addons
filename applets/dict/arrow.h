/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *   Copyright (C) 2007 by Jeff Cooper <weirdsox11@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsItem>
#include <QRectF>
#include <QPainter>

class Arrow : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        Arrow(QGraphicsItem *parent);
        ~Arrow();
        QRectF boundingRect() const;
        void setDirection(int dir); //0 is left, 1 is right TODO: replace with enum
    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        int m_direction;
    Q_SIGNALS:
        void clicked();
    protected:
        void mousePressEvent (QGraphicsSceneMouseEvent *event);

};
#endif
