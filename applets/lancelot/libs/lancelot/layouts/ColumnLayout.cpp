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

#include "ColumnLayout.h"
#include <QList>
#include <QGraphicsWidget>
#include <QGraphicsScene>

#define GOLDEN_SIZE  0.381966011250105  // 1 / (1 + phi); phi = (sqrt(5) + 1) / 2

namespace Lancelot
{

class GoldenColumnSizer: public ColumnLayout::ColumnSizer {
public:
    void init(int count) {
        m_position = 0;
        qreal size = 1.0;
        m_sizes.clear();
        while (count) {
            if (count == 1) {
                m_sizes.prepend(size);
            } else if (count == 2) {
                m_sizes.prepend((1 - GOLDEN_SIZE) * size);
                size *= GOLDEN_SIZE;
            } else {
                m_sizes.prepend(size * GOLDEN_SIZE);
                size -= size * GOLDEN_SIZE;
            }
            --count;
        }
    }

    qreal size() {
        if (m_position < 0 || m_position > m_sizes.size()) return 0;
        return m_sizes.at(m_position++);
    }
private:
    QList < qreal > m_sizes;
    int m_position;
};

class EqualColumnSizer: public ColumnLayout::ColumnSizer {
public:
    void init(int count) {
        m_count = count;
    }

    qreal size() {
        return 1.0 / m_count;
    }
private:
    int m_count;
};

ColumnLayout::ColumnSizer::~ColumnSizer()
{
}

ColumnLayout::ColumnSizer * ColumnLayout::ColumnSizer::create(SizerType type)
{
    switch (type) {
        case EqualSizer:
            return new EqualColumnSizer();
        case GoldenSizer:
            return new GoldenColumnSizer();
    }
    return NULL;
}

class ColumnLayout::Private {
public:
    ColumnLayout * q;
    QList < QGraphicsWidget * > items;
    QGraphicsItem * parentItem;
    ColumnLayout::ColumnSizer * sizer;
    int count;

    enum RelayoutType { Clean, Push, Pop, Resize };

    Private(ColumnLayout * parent)
        : q(parent), parentItem(NULL),
          sizer(new GoldenColumnSizer()), count(2) {}

    void _hide(QGraphicsWidget * widget) {
        // since Qt has some strange bug (or it
        // just doesn't behave as it should,
        // this is a temporary solution
        // so instead of hiding the item,
        // we are removing it from scene

        if (widget->parentItem()) {
            parentItem = widget->parentItem();
            widget->setParentItem(NULL);
            if (widget->scene()) {
                widget->scene()->removeItem(widget);
            }
        }
    }

    void _show(QGraphicsWidget * widget) {
        // see the comment in _hide

        if (!widget->parentItem()) {
            widget->setParentItem(parentItem);
        }
    }

    void relayout(RelayoutType type = Clean)
    {
        Q_UNUSED(type);
        if (items.size() == 0) return;

        int showItems   = qMin(items.size(), count);
        qreal width     = q->geometry().width();
        sizer->init(showItems);

        QRectF newGeometry = q->geometry();

        int i = 0;

        foreach (QGraphicsWidget * item, items) {
            if (items.size() - showItems > i++) {
                // item->setVisible(false);
                _hide(item);
            } else {
                qreal itemWidth = sizer->size() * width;
                if (itemWidth != 0) {
                    newGeometry.setWidth(itemWidth);
                    item->setGeometry(newGeometry);
                    _show(item);
                    newGeometry.moveLeft(newGeometry.left() + itemWidth);
                } else {
                    // item->setVisible(false);
                    _hide(item);
                }
            }
        }
    }

    void push(QGraphicsWidget * widget)
    {
        if (!widget || items.contains(widget)) {
            return;
        }

        //TODO: Uncomment : widget->hide();
        items.append(widget);

        q->updateGeometry();
    }

    QGraphicsWidget * pop()
    {
        QGraphicsWidget * widget = items.takeLast();
        relayout(Pop);
        return widget;
    }
};

void ColumnLayout::setSizer(ColumnLayout::ColumnSizer * sizer)
{
    delete d->sizer;
    d->sizer = sizer;
    d->relayout(Private::Resize);
}

ColumnLayout::ColumnSizer * ColumnLayout::sizer() const
{
    return d->sizer;
}

QSizeF ColumnLayout::sizeHint(Qt::SizeHint which,
        const QSizeF & constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    return QSizeF();
}

void ColumnLayout::push(QGraphicsWidget * widget)
{
    d->push(widget);
}

QGraphicsWidget * ColumnLayout::pop()
{
    return d->pop();
}

void ColumnLayout::setColumnCount(int count)
{
    if (count < 1) return;
    d->count = count;
    d->relayout(Private::Clean);
}

int ColumnLayout::columnCount() const
{
    return d->count;
}

ColumnLayout::ColumnLayout(QGraphicsLayoutItem * parent)
    : QGraphicsLayout(parent), d(new Private(this))
{

}

ColumnLayout::~ColumnLayout()
{
    delete d;
}

void ColumnLayout::removeAt(int index)
{
    Q_UNUSED(index);
}

QGraphicsLayoutItem * ColumnLayout::itemAt(int i) const
{
    return d->items.at(i);
}

int ColumnLayout::count() const
{
    return d->items.size();
}

void ColumnLayout::setGeometry(const QRectF & rect)
{
    QGraphicsLayout::setGeometry(rect);
    d->relayout();
}

} // namespace Lancelot

