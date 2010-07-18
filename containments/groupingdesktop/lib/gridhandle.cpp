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

#include "gridhandle.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <KDebug>
#include <KIconLoader>

#include <Plasma/Applet>
#include <Plasma/PaintUtils>

#include "groupingcontainment.h"
#include "abstractgroup.h"

GridHandle::GridHandle(GroupingContainment *containment, Plasma::Applet *applet)
          : Handle(containment, applet),
            m_moving(false)
{
    init();
}

GridHandle::GridHandle(GroupingContainment *containment, AbstractGroup *group)
          : Handle(containment, group),
            m_moving(false)
{
    init();
}

GridHandle::~GridHandle()
{

}

void GridHandle::init()
{
    QGraphicsWidget *w = widget();
    m_widgetPos = w->pos();
    m_widgetSize = w->size();
    m_maxWidgetSize = w->maximumSize();

    m_lastButton = Handle::NoButton;

    m_configureIcons = new Plasma::Svg(this);
    m_configureIcons->setImagePath("widgets/configuration-icons");
    m_configureIcons->setContainsMultipleImages(true);
}

void GridHandle::detachWidget()
{
    QGraphicsWidget *w = widget();
    w->setPos(m_widgetPos);
    w->setMaximumSize(m_maxWidgetSize);
    w->resize(m_widgetSize);

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
        if (boundingRect().x() < 0) { //horizontal
            widget()->setMaximumSize(m_maxWidgetSize.width() - 20, m_maxWidgetSize.height());
        } else {    //vertical
            widget()->setMaximumSize(m_maxWidgetSize.width(), m_maxWidgetSize.height() - 20);
        }
    }
}

void GridHandle::widgetResized()
{

}

void GridHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastButton = mapToButton(event->pos());
        if (m_lastButton == Handle::MoveButton) {
            m_startPos = event->pos();
        }
        event->accept();
        update();
    } else {
        event->ignore();
    }
}

void GridHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_lastButton == Handle::MoveButton) {
        if (!m_moving) {
            m_moving = true;
            containment()->setMovingWidget(widget());
        } else {
            QPointF p(event->pos() - m_startPos);
            widget()->moveBy(p.x(), p.y());
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void GridHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        m_lastButton = NoButton;
        event->ignore();
        return;
    }

    Handle::ButtonType button = mapToButton(event->pos());
    if (button != m_lastButton) {
        m_lastButton = NoButton;
        event->ignore();
        update();
        return;
    }

    switch (button) {
        case MoveButton:
            if (m_moving) {
                m_moving = false;
                emit widgetMoved(widget());
                m_widgetPos = widget()->pos();
            }

        break;

        case RemoveButton:
            if (applet()) {
                applet()->destroy();
            } else {
                group()->destroy();
            }

        break;

        case ConfigureButton:
            if (applet()) {
                applet()->showConfigurationInterface();
            } else {
                group()->showConfigurationInterface();
            }

        break;

        case MaximizeButton:
            if (applet()) {
                applet()->runAssociatedApplication();
            }

        break;

        default:
        break;
    }

    update();
}

void GridHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!boundingRect().contains(event->pos())) {
        emit disappearDone(this);
    }
}

Handle::ButtonType GridHandle::mapToButton(const QPointF &pos)
{
    if (enoughRoom()) {
        const QSize iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        QRectF iconRect(boundingRect().topLeft() + QPointF(2, 2), iconSize);
        bool horizontal = (iconRect.x() < 0);

        if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
            if (iconRect.contains(pos)) {
                return Handle::ConfigureButton;
            }

            if (horizontal) {
                iconRect.translate(0, iconSize.height());
            } else {
                iconRect.translate(iconSize.width(), 0);
            }
        }

        if (applet() && applet()->hasValidAssociatedApplication()) {
            if (iconRect.contains(pos)) {
                return Handle::MaximizeButton;
            }
        }

        if (horizontal) {
            iconRect.moveTo(boundingRect().bottomLeft() - QPointF(-2, iconSize.height() + 2));
        } else {
            iconRect.moveTo(boundingRect().topRight() - QPointF(iconSize.width() + 2, -2));
        }

        if (iconRect.contains(pos)) {
            return Handle::RemoveButton;
        }
    }

    return Handle::MoveButton;
}

bool GridHandle::enoughRoom()
{
    const QSize iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    bool horizontal = (boundingRect().x() < 0);

    int buttonsCount = 2;

    if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
        ++buttonsCount;
    }

    if (applet() && applet()->hasValidAssociatedApplication()) {
        ++buttonsCount;
    }

    return ((horizontal && boundingRect().height() > buttonsCount * iconSize.height()) ||
           (!horizontal && boundingRect().width() > buttonsCount * iconSize.width()));
}

void GridHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)

    QRectF rect = boundingRect();

    painter->setPen(Qt::NoPen);
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);

    QPainterPath p = Plasma::PaintUtils::roundedRectangle(rect.adjusted(1, 1, -2, -2), 4);
    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    c.setAlphaF(0.3);

    painter->fillPath(p, c);

    if (!enoughRoom()) {
        return;
    }

    QPointF shiftC;
    QPointF shiftD;
    QPointF shiftMx;

    switch(m_lastButton)
    {
        case ConfigureButton:
            shiftC = QPointF(2, 2);
            break;
        case RemoveButton:
            shiftD = QPointF(2, 2);
            break;
        case MaximizeButton:
            shiftMx = QPointF(2, 2);
            break;
        default:
            break;
    }

    const QSize iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    QRectF iconRect(boundingRect().topLeft() + QPointF(2, 2), iconSize);
    bool horizontal = (iconRect.x() < 0);

    if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
        m_configureIcons->paint(painter, iconRect.translated(shiftC), "configure");
        if (horizontal) {
            iconRect.translate(0, iconSize.height());
        } else {
            iconRect.translate(iconSize.width(), 0);
        }
    }

    if (applet() && applet()->hasValidAssociatedApplication()) {
        m_configureIcons->paint(painter, iconRect.translated(shiftMx), "maximize");
    }

    if (horizontal) {
        iconRect.moveTo(boundingRect().bottomLeft() - QPointF(-2, iconSize.height() + 2));
    } else {
        iconRect.moveTo(boundingRect().topRight() - QPointF(iconSize.width() + 2, -2));
    }
    m_configureIcons->paint(painter, iconRect.translated(shiftD), "close");
}

#include "gridhandle.moc"
