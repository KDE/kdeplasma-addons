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

#include <plasma/theme.h>

#include "arrow.h"

Arrow::Arrow(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    m_direction = 0;
}

Arrow::~Arrow()
{
}

QRectF Arrow::boundingRect() const
{
    return QRectF(0,0,10,30);
}

void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    QColor penColor = Plasma::Theme::self()->textColor();
    penColor.setAlpha(200);
    painter->setPen(QPen(QBrush(penColor), 1,Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush(Plasma::Theme::self()->textColor(), Qt::SolidPattern));
    if (m_direction == 0) {
        QPoint points[3] = {QPoint(10,0),
            QPoint(0,15),
                   QPoint(10,30)};
                   painter->drawPolygon(points, 3);
    } else {
        QPoint points[3] = {QPoint(0,0),
            QPoint(10,15),
                   QPoint(0,30)};
                   painter->drawPolygon(points, 3);
    }
}

void Arrow::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    emit clicked();
}

void Arrow::setDirection(int dir)
{
    m_direction = dir;
}

#include "arrow.moc"
