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

#include "appletoverlay.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <KIcon>

#include <Plasma/Applet>
#include <QtCore/QTimer>

AppletOverlay::AppletOverlay(Plasma::Applet *applet, Qt::WindowFlags wFlags)
             : QGraphicsWidget(applet, wFlags),
               m_applet(applet),
               m_moving(false)
{
    setAcceptHoverEvents(true);

    setZValue(m_applet->zValue() + 1);
    syncGeometry();

    connect(applet, SIGNAL(geometryChanged()), this, SLOT(delayedSyncGeometry()));
}

AppletOverlay::~AppletOverlay()
{
    m_applet->setZValue(m_savedZValue);
}

void AppletOverlay::setZ(int value)
{
    m_savedZValue = m_applet->zValue();
    m_applet->setZValue(value);
}

Plasma::Applet *AppletOverlay::applet() const
{
    return m_applet;
}

void AppletOverlay::delayedSyncGeometry()
{
    QTimer::singleShot(0, this, SLOT(syncGeometry()));
}

void AppletOverlay::syncGeometry()
{
    setGeometry(m_applet->contentsRect());
}

bool AppletOverlay::isMoving() const
{
    return m_moving;
}

void AppletOverlay::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_moving) {
            m_moving = false;
            emit endMoving();
        } else {
            m_moving = true;
            m_startPos = event->pos();
            emit startMoving();
        }
    }
}

void AppletOverlay::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_moving) {
        QPointF p(event->pos() - m_startPos);
        emit movedOf(p.x(), p.y());
    }
}

void AppletOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_moving) {
        QPointF p(event->pos() - m_startPos);
        emit movedOf(p.x(), p.y());
    }
}

void AppletOverlay::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_moving) {
        QPointF p(event->pos() - m_startPos);
        emit appletMovedOutside(p.x(), p.y());
    }
}

void AppletOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    KIcon icon("transform-move");

    int iconSize = qMin(qMin((int)geometry().height(), int(m_applet->size().width())), 64);
    QRect iconRect(rect().center().toPoint() - QPoint(iconSize / 2, iconSize / 2), QSize(iconSize, iconSize));

    painter->drawPixmap(iconRect, icon.pixmap(iconSize, iconSize));
}

#include "appletoverlay.moc"