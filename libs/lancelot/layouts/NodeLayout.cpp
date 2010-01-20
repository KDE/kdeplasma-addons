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

#include "NodeLayout.h"
#include "Global.h"

#include <QMap>
#include <QPair>

#include <lancelot/lancelot.h>

#include <limits>

namespace Lancelot
{

NodeLayout::NodeCoordinate::NodeCoordinate(qreal xRelative, qreal yRelative, qreal xAbsolute, qreal yAbsolute)
    : xr(xRelative), xa(xAbsolute), yr(yRelative), ya(yAbsolute) {}

NodeLayout::NodeCoordinate NodeLayout::NodeCoordinate::simple(qreal x, qreal y,
        CoordinateType xType, CoordinateType yType)
{
    NodeLayout::NodeCoordinate coo;
    switch (xType) {
    case Relative:
        coo.xr = x;
        coo.xa = 0;
        break;
    case Absolute:
        coo.xr = 0;
        coo.xa = x;
        break;
    case InnerRelative:
        coo.xr = x;
        coo.xa = QREAL_INFINITY;
        break;
    }

    switch (yType) {
    case Relative:
        coo.yr = y;
        coo.ya = 0;
        break;
    case Absolute:
        coo.yr = 0;
        coo.ya = y;
        break;
    case InnerRelative:
        coo.yr = y;
        coo.ya = QREAL_INFINITY;
        break;
    }
    return coo;
}

class NodeLayout::Private {
public:
    QMap <QGraphicsLayoutItem * , QPair < NodeCoordinate, NodeCoordinate > > items;
    NodeLayout * parent;
    QMap < Qt::SizeHint, QSizeF > sizeHintCache;

    Private(NodeLayout * parentLayout) {
        parent = parentLayout;
    }

    void relayout()
    {
        foreach (QGraphicsLayoutItem * item, items.keys()) // krazy:exclude=foreach
        {
            if (item) {
                item->setGeometry(calculateRectangle(item));
            }
        }
    }

    qreal calculateXPosition(const NodeCoordinate & coo, const QRectF & parentGeometry) const
    {
        return parentGeometry.left() + (coo.xr * parentGeometry.width())  + coo.xa;
    }

    qreal calculateYPosition(const NodeCoordinate & coo, const QRectF & parentGeometry) const
    {
        return parentGeometry.top() + (coo.yr * parentGeometry.height())  + coo.ya;
    }

    QPointF calculatePosition(const NodeCoordinate & coo, const QRectF & parentGeometry) const
    {
        Q_UNUSED( parentGeometry );

        return QPointF(
            calculateXPosition(coo, parent->geometry()),
            calculateYPosition(coo, parent->geometry())
        );
    }

    QRectF calculateRectangle(QGraphicsLayoutItem * item, QRectF geometry = QRectF()) const
    {
        if (geometry == QRectF()) {
            geometry = parent->geometry();
        }

        QRectF result;
        if (!item || !items.contains(item)) {
            return QRectF();
        }

        result.setTopLeft(calculatePosition(items[item].first, geometry));

        if (items[item].second.xa != QREAL_INFINITY) {
            result.setRight(calculateXPosition(items[item].second, geometry));
        } else {
            result.setWidth(item->preferredSize().width());
            result.moveLeft(result.left() - items[item].second.xr * result.width());
        }

        if (items[item].second.ya != QREAL_INFINITY) {
            result.setBottom(calculateYPosition(items[item].second, geometry));
        } else {
            result.setHeight(item->preferredSize().height());
            result.moveTop(result.top() - items[item].second.yr * result.height());
        }
        return result;
    }

    void calculateSizeHint(QGraphicsLayoutItem * item = NULL) {
        if (item == NULL) {
            // Recalculate the sizeHint using all items
            sizeHintCache[Qt::MinimumSize] = QSizeF();
            sizeHintCache[Qt::MaximumSize] = QSizeF();
            sizeHintCache[Qt::PreferredSize] = QSizeF();
            foreach (QGraphicsLayoutItem * item, items.keys()) // krazy:exclude=foreach
            {
                if (item) {
                    calculateSizeHint(item);
                }
            }
        } else {
            // Calculate size hint for current item
            const QRectF scaled = calculateRectangle(item, QRectF(0, 0, 1, 1));

            // qMin(..., 1.0) so that for autosized elements we don't get smaller
            // size than the item's size itself. The sizeHint for NodeLayout can
            // not do anything smarter concerning the sizeHint when there are
            // autosized elements.

            QSizeF size;

            foreach (const Qt::SizeHint &which, sizeHintCache.keys()) // krazy:exclude=foreach
            {
                size = item->effectiveSizeHint(which);
                size.scale(
                    1 / qMin(scaled.width(), qreal(1.0)),
                    1 / qMin(scaled.height(), qreal(1.0)),
                    Qt::IgnoreAspectRatio
                );
                sizeHintCache[which] = sizeHintCache[which].expandedTo(size);
            }
        }
    }

};

NodeLayout::NodeLayout(QGraphicsLayoutItem * parent)
  : QGraphicsLayout(parent), d(new Private(this))
{
}

NodeLayout::~NodeLayout()
{
    delete d;
}

QSizeF NodeLayout::sizeHint(Qt::SizeHint which,
        const QSizeF & constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    QSizeF result;

    switch (which) {
        case Qt::MaximumSize:
            result = MAX_WIDGET_SIZE;
            break;
        default:
            result = d->sizeHintCache[which];
    }
    if (constraint.isValid()) {
        result = result.boundedTo(constraint);
    }
    return result;
}

void NodeLayout::addItem(QGraphicsLayoutItem * item)
{
    NodeLayout::addItem(item, NodeCoordinate());
}

void NodeLayout::addItem(QGraphicsLayoutItem * item, NodeCoordinate topLeft, NodeCoordinate bottomRight)
{
    if (!item) {
        return;
    }

    d->items[item] = QPair<NodeCoordinate, NodeCoordinate>(topLeft, bottomRight);
    d->calculateSizeHint(item);
    updateGeometry();
}

void NodeLayout::addItem(QGraphicsLayoutItem * item, NodeCoordinate node, qreal xr, qreal yr)
{
    if (!item) {
        return;
    }

    d->items[item] = QPair<NodeCoordinate, NodeCoordinate>(node,
        NodeCoordinate::simple(xr, yr, NodeCoordinate::InnerRelative, NodeCoordinate::InnerRelative));
    d->calculateSizeHint(item);
    updateGeometry();
}

int NodeLayout::count() const
{
    return d->items.count();
}

QGraphicsLayoutItem * NodeLayout::itemAt(int i) const
{
    if (i >= d->items.count()) {
        return 0;
    }

    return d->items.keys()[i];
}

void NodeLayout::removeAt(int i)
{
    if (i >= d->items.count()) {
        return;
    }

    d->items.remove(itemAt(i));
}

void NodeLayout::setGeometry(const QRectF & rect)
{
    QGraphicsLayout::setGeometry(rect);
    d->relayout();
}

} // namespace Lancelot

