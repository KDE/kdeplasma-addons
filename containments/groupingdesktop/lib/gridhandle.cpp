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

#include "gridhandle.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsLinearLayout>

#include <KDebug>
#include <KIcon>

#include <Plasma/Applet>

GridHandle::GridHandle(GroupingContainment *containment, Plasma::Applet *applet)
          : Handle(containment, applet),
            m_moving(false)
{
//     setGeometry(applet->contentsRect());
}

GridHandle::GridHandle(GroupingContainment *containment, AbstractGroup *group)
          : Handle(containment, group),
            m_moving(false)
{
//     setGeometry(applet->contentsRect());
}

GridHandle::~GridHandle()
{

}

void GridHandle::delayedSyncGeometry()
{
    QTimer::singleShot(0, this, SLOT(syncGeometry()));
}

void GridHandle::syncGeometry()
{
//     setGeometry(m_item->contentsRect());
}

void GridHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_moving) {
            m_moving = false;
        } else {
            m_moving = true;
            m_startPos = event->pos();
        }
    }
}

void GridHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_moving) {
        QPointF p(event->pos() - m_startPos);
        widget()->moveBy(p.x(), p.y());
    }
}

void GridHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_moving = false;
}

void GridHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    KIcon icon("transform-move");

//     int iconSize = qMin(qMin((int)geometry().height(), int(m_item->size().width())), 64);
    int iconSize = 20;
//     QRect iconRect(rect().center().toPoint() - QPoint(iconSize / 2, iconSize / 2), QSize(iconSize, iconSize));

//     painter->drawPixmap(iconRect, icon.pixmap(iconSize, iconSize));
}

#include "gridhandle.moc"
