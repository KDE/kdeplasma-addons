/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Widget.h"
#include "Global.h"

#include <QApplication>
#include <QVariant>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>

#include <lancelot/lancelot.h>

namespace Lancelot {

class Widget::Private {
public:
    Private()
      : hover(false),
        down(false),
        paintBackwards(false)
    {
    };

    bool hover : 1;
    bool down : 1;
    bool paintBackwards : 1;
};

Widget::Widget(QGraphicsItem * parent)
  : QGraphicsWidget(parent),
    d(new Private())
{
    setGroup(NULL);
    setFlag(QGraphicsItem::ItemClipsToShape);
}

Widget::~Widget()
{
    delete d;
}

void Widget::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    if (d->hover) {
        return ;
    }
    if (!isEnabled()) return;
    d->hover = true;
    QGraphicsWidget::hoverEnterEvent(event);
    emit mouseHoverEnter();
    update();
}

void Widget::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    if (!(d->hover)) {
        return ;
    }
    if (!isEnabled()) return;
    d->hover = false;
    QGraphicsWidget::hoverLeaveEvent(event);
    emit mouseHoverLeave();
    update();
}

void Widget::setHovered(bool value)
{
    if (d->hover == value) {
        return ;
    }
    d->hover = value;
    update();
}

void Widget::setDown(bool value)
{
    d->down = value;
    update();
}

void Widget::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        d->down = true;
        update();
        event->accept();
        emit pressed();
    } else {
        QGraphicsWidget::mousePressEvent(event);
    }
}

void Widget::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (d->down && (event->button() == Qt::LeftButton)) {
        d->down = false;
        emit released();
        emit clicked();
    } else {
        QGraphicsWidget::mouseReleaseEvent(event);
    }
}

bool Widget::isHovered() const
{
    return d->hover;
}

QString Widget::groupName() const
{
    if (group()) {
        return group()->name();
    }
    return QString();
}

void Widget::setGroupByName(const QString & groupName)
{
    setGroup(Global::self()->group(groupName));
}

void Widget::setGroup(Group * group)
{
    Global::self()->setGroupForObject(this, group);
}

Group * Widget::group() const
{
    return Global::self()->groupForObject(
        const_cast < Widget * > (this));
}

void Widget::paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
        QWidget * widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    paintBackground(painter);
}

void Widget::paintBackground(QPainter * painter)
{
    if (!group()) return;

    QString element;
    if (!isEnabled()) {
        element = "disabled";
    } else if (d->down) {
        element = "down";
    } else if (d->hover) {
        element = "active";
    }

    paintBackground(painter, element);
}

void Widget::paintBackground(QPainter * painter, const QString & element)
{
    if (!group()) return;
    bool rtl = QApplication::isRightToLeft();

    // Svg Background Painting
    if (Plasma::FrameSvg * svg = group()->backgroundSvg()) {
        svg->setElementPrefix(element);
        if (svg->size() != size().toSize()) {
            svg->resizeFrame(size().toSize());
        }

        if (rtl && d->paintBackwards) {
            painter->translate(size().width(), 0);
            painter->scale(-1, 1);
        }
        svg->paintFrame(painter);
        if (rtl && d->paintBackwards) {
            painter->translate(size().width(), 0);
            painter->scale(-1, 1);
        }

        return;
    }

    // If we don't have SVG, should we paint a rectangle?
    if (!group()->hasProperty("WholeColorBackground")) {
        return;
    }

    if (const Group::ColorScheme * scheme = group()->backgroundColor()) {
        const QColor * color;
        if (!isEnabled()) {
            color = & (scheme->disabled);
        } else if (d->hover) {
            color = & (scheme->active);
        } else {
            color = & (scheme->normal);
        }
        painter->fillRect(QRectF(QPointF(0, 0), size()), QBrush(*color));
    }
}

QSizeF Widget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF result;
    if (!group()) return result;

    switch (which) {
        case Qt::MinimumSize:
            result = QSizeF();
            break;
        case Qt::MaximumSize:
            result = MAX_WIDGET_SIZE;
            break;
        case Qt::PreferredSize:
            if (Plasma::FrameSvg * svg = group()->backgroundSvg()) {
                result = QSizeF(
                    svg->marginSize(Plasma::LeftMargin) +
                    svg->marginSize(Plasma::RightMargin),
                    svg->marginSize(Plasma::TopMargin) +
                    svg->marginSize(Plasma::BottomMargin)
                    );
            } else {
                result = QSizeF();
            }
            break;
        default:
            result = QSizeF();
            break;
    }
    if (constraint.isValid()) {
        result = result.boundedTo(constraint);
    }
    return result;
}

bool Widget::isDown() const
{
    return d->down;
}

void Widget::hideEvent(QHideEvent * event)
{
    d->down = false;
    d->hover = false;
    QGraphicsWidget::hideEvent(event);
}

void Widget::setPaintBackwardsWhenRTL(bool value)
{
    d->paintBackwards = value;
}


} // namespace Lancelot

#include "Widget.moc"

