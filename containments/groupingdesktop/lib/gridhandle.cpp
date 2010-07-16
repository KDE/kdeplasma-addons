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
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <KDebug>
#include <KIcon>

#include <Plasma/Applet>
#include <Plasma/PaintUtils>

#include "groupingcontainment.h"
#include "abstractgroup.h"

GridHandle::GridHandle(GroupingContainment *containment, Plasma::Applet *applet)
          : Handle(containment, applet),
            m_moving(false)
{
    setZValue(-1000);
    m_widgetPos = applet->pos();
}

GridHandle::GridHandle(GroupingContainment *containment, AbstractGroup *group)
          : Handle(containment, group),
            m_moving(false)
{
    setZValue(-1000);
    m_widgetPos = group->pos();
}

GridHandle::~GridHandle()
{

}

void GridHandle::detachWidget()
{
    widget()->setPos(m_widgetPos);

    Handle::detachWidget();
}

QRectF GridHandle::boundingRect() const
{
    QRectF rect(widget()->boundingRect());
    if (rect.width() >= rect.height()) {
        return QRectF(-20, 0, 20, rect.height());
    } else {
        return QRectF(0, -20, rect.width(), 20);
    }
}

void GridHandle::setHoverPos(const QPointF &hoverPos)
{
    if (!widget()->boundingRect().contains(hoverPos)) {
        emit disappearDone(this);
    } else {
        widget()->setPos(m_widgetPos - boundingRect().topLeft());
    }
}

void GridHandle::widgetResized()
{

}

void GridHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_startPos = event->pos();
        event->accept();
    }
}

void GridHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_moving) {
        m_moving = true;
        containment()->setMovingWidget(widget());
    } else {
        QPointF p(event->pos() - m_startPos);
        widget()->moveBy(p.x(), p.y());
    }
}

void GridHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_moving = false;
    emit widgetMoved(widget());

    m_widgetPos = widget()->pos();
}

void GridHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!boundingRect().contains(event->pos())) {
        emit disappearDone(this);
    }
}

void GridHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)

    QRectF rect = boundingRect();

    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath p = Plasma::PaintUtils::roundedRectangle(rect.adjusted(1, 1, -2, -2), 4);
    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    c.setAlphaF(0.3);

    painter->fillPath(p, c);

    KIcon icon("transform-move");

    int iconSize = qMin(qMin((int)rect.height(), int(rect.size().width())), 64);
    QRect iconRect(rect.center().toPoint() - QPoint(iconSize / 2, iconSize / 2), QSize(iconSize, iconSize));

    painter->drawPixmap(iconRect, icon.pixmap(iconSize, iconSize));
}

#include "gridhandle.moc"
