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

#include "itemoverlay.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsLinearLayout>

#include <KDebug>
#include <KIcon>

#include <QtCore/QTimer>

ItemOverlay::ItemOverlay(QGraphicsWidget *item, Qt::WindowFlags wFlags)
             : QGraphicsWidget(item, wFlags),
               m_item(item),
//                m_layout(new QGraphicsLinearLayout(m_item)),
               m_moving(false)
{
    setAcceptHoverEvents(true);

        setGeometry(m_item->contentsRect());
//     m_layout->addItem(this);
}

ItemOverlay::~ItemOverlay()
{
    m_item->setZValue(m_savedZValue);
}

void ItemOverlay::setZ(int value)
{
    m_savedZValue = m_item->zValue();
    m_item->setZValue(value);
}

QGraphicsWidget *ItemOverlay::item() const
{
    return m_item;
}

void ItemOverlay::delayedSyncGeometry()
{
    QTimer::singleShot(0, this, SLOT(syncGeometry()));
}

void ItemOverlay::syncGeometry()
{
    setGeometry(m_item->contentsRect());
}

bool ItemOverlay::isMoving() const
{
    return m_moving;
}

void ItemOverlay::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_moving) {
            m_moving = false;
//             emit endMoving();
        } else {
            m_moving = true;
            m_startPos = event->pos();
//             emit startMoving();
        }
    }
}

void ItemOverlay::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_moving) {
        QPointF p(event->pos() - m_startPos);
//         emit movedOf(p.x(), p.y(), event->pos());
    }
}

void ItemOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_moving) {
        QPointF p(event->pos() - m_startPos);
//         kDebug()<<geometry()<<event->pos();
//         emit movedOf(p.x(), p.y(), event->pos());
    }
}

void ItemOverlay::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_moving) {
        QPointF p(event->pos() - m_startPos);
//         kDebug()<<geometry()<<event->pos();
//         emit itemMovedOutside(p.x(), p.y());
    }
}

void ItemOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    KIcon icon("transform-move");

    int iconSize = qMin(qMin((int)geometry().height(), int(m_item->size().width())), 64);
    QRect iconRect(rect().center().toPoint() - QPoint(iconSize / 2, iconSize / 2), QSize(iconSize, iconSize));

    painter->drawPixmap(iconRect, icon.pixmap(iconSize, iconSize));
}

#include "itemoverlay.moc"
