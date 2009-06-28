/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "FullBorderLayout.h"
#include <lancelot/lancelot.h>
#include <QGraphicsWidget>

namespace Lancelot {

class FullBorderLayout::Private {
public:
    FullBorderLayout * q;

    explicit Private(FullBorderLayout * parent = 0)
        : q(parent)
    {
        sizes[FullBorderLayout::LeftBorder] = -1;
        sizes[FullBorderLayout::RightBorder] = -1;
        sizes[FullBorderLayout::TopBorder] = -1;
        sizes[FullBorderLayout::BottomBorder] = -1;
    }

    virtual ~Private()
    {
    }

    QMap < FullBorderLayout::Place, QGraphicsLayoutItem * > itemPositions;
    QMap < FullBorderLayout::Border, qreal > sizes;

    void calculateBorderSizes(qreal & top, qreal & bottom, qreal & left, qreal & right) // const
    {
        // top
        top = sizes[FullBorderLayout::TopBorder];
        if (top < 0) {
            top = 0;
            if (itemPositions[FullBorderLayout::TopLeft]) {
                top = qMax(top, itemPositions[FullBorderLayout::TopLeft]->preferredSize().height());
            }
            if (itemPositions[FullBorderLayout::Top]) {
                top = qMax(top, itemPositions[FullBorderLayout::Top]->preferredSize().height());
            }
            if (itemPositions[FullBorderLayout::TopRight]) {
                top = qMax(top, itemPositions[FullBorderLayout::TopRight]->preferredSize().height());
            }
        }

        // bottom
        bottom = sizes[FullBorderLayout::BottomBorder];
        if (bottom < 0) {
            bottom = 0;
            if (itemPositions[FullBorderLayout::BottomLeft]) {
                bottom = qMax(bottom, itemPositions[FullBorderLayout::BottomLeft]->preferredSize().height());
            }
            if (itemPositions[FullBorderLayout::Bottom]) {
                bottom = qMax(bottom, itemPositions[FullBorderLayout::Bottom]->preferredSize().height());
            }
            if (itemPositions[FullBorderLayout::BottomRight]) {
                bottom = qMax(bottom, itemPositions[FullBorderLayout::BottomRight]->preferredSize().height());
            }
        }

        // left
        left = sizes[FullBorderLayout::LeftBorder];
        if (left < 0) {
            left = 0;
            if (itemPositions[FullBorderLayout::TopLeft]) {
                left = qMax(left, itemPositions[FullBorderLayout::TopLeft]->preferredSize().width());
            }
            if (itemPositions[FullBorderLayout::Left]) {
                left = qMax(left, itemPositions[FullBorderLayout::Left]->preferredSize().width());
            }
            if (itemPositions[FullBorderLayout::BottomLeft]) {
                left = qMax(left, itemPositions[FullBorderLayout::BottomLeft]->preferredSize().width());
            }
        }

        // right
        right = sizes[FullBorderLayout::RightBorder];
        if (right < 0) {
            right = 0;
            if (itemPositions[FullBorderLayout::TopRight]) {
                right = qMax(right, itemPositions[FullBorderLayout::TopRight]->preferredSize().width());
            }
            if (itemPositions[FullBorderLayout::Right]) {
                right = qMax(right, itemPositions[FullBorderLayout::Right]->preferredSize().width());
            }
            if (itemPositions[FullBorderLayout::BottomRight]) {
                right = qMax(right, itemPositions[FullBorderLayout::BottomRight]->preferredSize().width());
            }
        }
    }
};

FullBorderLayout::FullBorderLayout(QGraphicsLayoutItem * parent) :
    QGraphicsLayout(parent), d(new Private(this))
{
}

FullBorderLayout::~FullBorderLayout()
{
    delete d;
}

void FullBorderLayout::setGeometry(const QRectF & rect)
{
    QGraphicsLayout::setGeometry(rect);

    QRectF effectiveRect = geometry();
    qreal left = 0, top = 0, right = 0, bottom = 0;
    getContentsMargins(&left, &top, &right, &bottom);
    effectiveRect.adjust(+left, +top, -right, -bottom);

    qreal topBorder, bottomBorder, leftBorder, rightBorder;
    d->calculateBorderSizes(topBorder, bottomBorder, leftBorder, rightBorder);

    QRectF itemRect;

    itemRect = effectiveRect;
    itemRect.setSize(QSizeF(leftBorder, topBorder));

    if (d->itemPositions[TopLeft]) {
        d->itemPositions[TopLeft]->setGeometry(itemRect);
    }

    itemRect.setLeft(effectiveRect.left() + leftBorder);
    itemRect.setWidth(effectiveRect.width() - leftBorder - rightBorder);

    if (d->itemPositions[Top]) {
        d->itemPositions[Top]->setGeometry(itemRect);
    }

    itemRect.setWidth(rightBorder);
    itemRect.moveRight(effectiveRect.right());

    if (d->itemPositions[TopRight]) {
        d->itemPositions[TopRight]->setGeometry(itemRect);
    }

    itemRect.setTop(effectiveRect.top() + topBorder);
    itemRect.setHeight(effectiveRect.height() - topBorder - bottomBorder);

    if (d->itemPositions[Right]) {
        d->itemPositions[Right]->setGeometry(itemRect);
    }

    itemRect.setHeight(bottomBorder);
    itemRect.moveBottom(effectiveRect.bottom());

    if (d->itemPositions[BottomRight]) {
        d->itemPositions[BottomRight]->setGeometry(itemRect);
    }

    itemRect.setLeft(effectiveRect.left() + leftBorder);
    itemRect.setWidth(effectiveRect.width() - leftBorder - rightBorder);

    if (d->itemPositions[Bottom]) {
        d->itemPositions[Bottom]->setGeometry(itemRect);
    }

    itemRect.setLeft(effectiveRect.left());
    itemRect.setWidth(leftBorder);

    if (d->itemPositions[BottomLeft]) {
        d->itemPositions[BottomLeft]->setGeometry(itemRect);
    }

    itemRect.setTop(effectiveRect.top() + topBorder);
    itemRect.setHeight(effectiveRect.height() - topBorder - bottomBorder);

    if (d->itemPositions[Left]) {
        d->itemPositions[Left]->setGeometry(itemRect);
    }

    itemRect = effectiveRect;
    itemRect.adjust(
            leftBorder, topBorder,
            - rightBorder, - bottomBorder
            );

    if (d->itemPositions[Center]) {
        d->itemPositions[Center]->setGeometry(itemRect);
    }
}

QSizeF FullBorderLayout::sizeHint(Qt::SizeHint which,
        const QSizeF & constraint) const
{
    if (which == Qt::MaximumSize) {
        return MAX_WIDGET_SIZE;
    }

    qreal topMargin, bottomMargin, leftMargin, rightMargin;
    d->calculateBorderSizes(topMargin, bottomMargin, leftMargin, rightMargin);

    qreal hintHeight = topMargin + bottomMargin;
    qreal hintWidth  = leftMargin + rightMargin;

    if (d->itemPositions[Center]) {
        hintHeight += d->itemPositions[Center]
            ->effectiveSizeHint(which, constraint).height();
        hintWidth += d->itemPositions[Center]
            ->effectiveSizeHint(which, constraint).width();
    }

    QSizeF result = QSizeF(hintWidth, hintHeight);
    if (constraint != QSizeF(-1, -1)) {
        result = result.boundedTo(constraint);
    }
    return result;
}

void FullBorderLayout::addItem(QGraphicsLayoutItem * item)
{
    FullBorderLayout::addItem (item, Center);
}

void FullBorderLayout::addItem(QGraphicsLayoutItem * item, Place position)
{
    d->itemPositions[position] = item;
    updateGeometry();
}

int FullBorderLayout::count() const
{
    int count = 0;
    foreach (QGraphicsLayoutItem * i, d->itemPositions) {
        if (i) {
            ++count;
        }
    }
    return count;
}

QGraphicsLayoutItem * FullBorderLayout::itemAt(int index) const
{
    int count = 0;
    foreach (QGraphicsLayoutItem * i, d->itemPositions) {
        if (i) {
            if (index == count) {
                return i;
            }
            count++;
        }
    }
    return 0;
}

void FullBorderLayout::setSize(qreal size, Border border)
{
    d->sizes[border] = size;
    updateGeometry();
}

void FullBorderLayout::setAutoSize(Border border)
{
    d->sizes[border] = -1;
    updateGeometry();
}

qreal FullBorderLayout::size(Border border) const
{
    return d->sizes[border];
}

void FullBorderLayout::removeAt(int index)
{
    QMutableMapIterator < Place, QGraphicsLayoutItem * > i(d->itemPositions);
    int count = 0;
    while (i.hasNext()) {
        i.next();
        if (i.value()) {
            if (index == count++) {
                i.remove();
            }
        }
    }
    updateGeometry();
}

} // namespace Lancelot

