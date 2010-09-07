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
            m_moving(false),
            SIZE(20)
{
    init();
}

GridHandle::GridHandle(GroupingContainment *containment, AbstractGroup *group)
          : Handle(containment, group),
            m_moving(false),
            SIZE(20)
{
    init();
}

GridHandle::~GridHandle()
{

}

void GridHandle::init()
{
    m_location = NoLocation;

    QGraphicsWidget *w = widget();
    m_widgetPos = w->pos();
    m_widgetSize = w->size();
    w->installEventFilter(this);

    m_lastButton = Handle::NoButton;

    m_configureIcons = new Plasma::Svg(this);
    m_configureIcons->setImagePath("widgets/configuration-icons");
    m_configureIcons->setContainsMultipleImages(true);
}

void GridHandle::detachWidget()
{
    QGraphicsWidget *w = widget();
    w->removeEventFilter(this);
    w->setPos(m_widgetPos);

    Handle::detachWidget();
}

bool GridHandle::eventFilter(QObject *obj, QEvent *event)
{
    QGraphicsWidget *w = qobject_cast<QGraphicsWidget *>(obj);

    if (w != widget()) {
        return true;
    }

    if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        if (!fullRect().contains(static_cast<QGraphicsSceneHoverEvent *>(event)->pos())) {
            emit disappearDone(this);
        }
    } else if (event->type() == QEvent::GraphicsSceneMove) {
        QGraphicsSceneMoveEvent *e = static_cast<QGraphicsSceneMoveEvent *>(event);
        m_widgetPos = e->newPos();
    }

    return false;
}

QRectF GridHandle::boundingRect() const
{
    switch (m_location) {
        case Left:
            return QRectF(-SIZE, 0, SIZE, m_widgetSize.height());

        case Top:
            return QRectF(0, -SIZE, m_widgetSize.width(), SIZE);

        case Right:
            return QRectF(m_widgetSize.width(), 0, SIZE, m_widgetSize.height());

        case Bottom:
            return QRectF(0, m_widgetSize.height(), m_widgetSize.width(), SIZE);

        default:
            return QRectF();
    }
}

void GridHandle::setHoverPos(const QPointF &hoverPos)
{
    if (isHorizontal()) {
        const qreal width = m_widgetSize.width() / 2.;
        QRectF left(0, 0, width, m_widgetSize.height());
        QRectF right(width, 0, width, m_widgetSize.height());
        if (left.contains(hoverPos)) {
            m_location = Left;
        } else if (right.contains(hoverPos)) {
            m_location = Right;
        }
    } else {
        const qreal height = m_widgetSize.height() / 2.;
        QRectF top(0, 0, m_widgetSize.width(), height);
        QRectF bottom(0, height, m_widgetSize.width(), height);
        if (top.contains(hoverPos)) {
            m_location = Top;
        } else if (bottom.contains(hoverPos)) {
            m_location = Bottom;
        }
    }

    widget()->removeEventFilter(this);
    switch (m_location) {
        case Left:
            widget()->setPos(m_widgetPos + QPointF(SIZE, 0));
        break;

        case Top:
            widget()->setPos(m_widgetPos + QPointF(0, SIZE));
        break;

        case Right:
            widget()->setPos(m_widgetPos + QPointF(-SIZE, 0));
        break;

        case Bottom:
            widget()->setPos(m_widgetPos + QPointF(0, -SIZE));
        break;

        default:
            emit disappearDone(this);
            return;
    }
    widget()->installEventFilter(this);
}

bool GridHandle::isHorizontal() const
{
    QRectF rect(widget()->parentItem()->boundingRect());
    return rect.width() >= rect.height();
}

void GridHandle::widgetResized()
{
    m_widgetSize = widget()->size();
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

QRectF GridHandle::fullRect() const
{
    return QRectF(boundingRect().topLeft(), widget()->boundingRect().bottomRight());
}

void GridHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!fullRect().contains(event->pos())) {
        emit disappearDone(this);
    }
}

Handle::ButtonType GridHandle::mapToButton(const QPointF &pos)
{
    if (enoughRoom()) {
        const QSize iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        QRectF iconRect(boundingRect().topLeft() + QPointF(2, 2), iconSize);

        if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
            if (iconRect.contains(pos)) {
                return Handle::ConfigureButton;
            }

            if (isHorizontal()) {
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

        if (isHorizontal()) {
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
    int buttonsCount = 2;

    if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
        ++buttonsCount;
    }

    if (applet() && applet()->hasValidAssociatedApplication()) {
        ++buttonsCount;
    }

    return ((isHorizontal() && boundingRect().height() > buttonsCount * iconSize.height()) ||
           (!isHorizontal() && boundingRect().width() > buttonsCount * iconSize.width()));
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

    if ((applet() && applet()->hasConfigurationInterface()) || (group() && group()->hasConfigurationInterface())) {
        m_configureIcons->paint(painter, iconRect.translated(shiftC), "configure");
        if (isHorizontal()) {
            iconRect.translate(0, iconSize.height());
        } else {
            iconRect.translate(iconSize.width(), 0);
        }
    }

    if (applet() && applet()->hasValidAssociatedApplication()) {
        m_configureIcons->paint(painter, iconRect.translated(shiftMx), "maximize");
    }

    if (isHorizontal()) {
        iconRect.moveTo(boundingRect().bottomLeft() - QPointF(-2, iconSize.height() + 2));
    } else {
        iconRect.moveTo(boundingRect().topRight() - QPointF(iconSize.width() + 2, -2));
    }
    m_configureIcons->paint(painter, iconRect.translated(shiftD), "close");
}

#include "gridhandle.moc"
