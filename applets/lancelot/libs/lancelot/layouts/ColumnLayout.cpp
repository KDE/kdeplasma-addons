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
#include <plasma/layouts/layoutitem.h>

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

class ColumnLayout::Private: public QObject {
    Q_OBJECT
public Q_SLOTS:
    void stateChanged(
            Plasma::LayoutItem * item,
            Plasma::LayoutAnimator::State oldState,
            Plasma::LayoutAnimator::State newState
    )
    {
        if (oldState == Plasma::LayoutAnimator::RemovedState
                && items.contains((Plasma::Widget *)item)) {
            ((Plasma::Widget *)item)->hide();
        }
    }

public:
    ColumnLayout * q;
    QList < Plasma::Widget * > items;
    Plasma::LayoutAnimator * animator;
    ColumnLayout::ColumnSizer * sizer;
    int count;

    enum RelayoutType { Clean, Push, Pop, Resize };

    Private(ColumnLayout * parent)
        : q(parent), animator(NULL), count(2), sizer(new GoldenColumnSizer()) {}

    void relayout(RelayoutType type = Clean)
    {
        if (items.size() == 0) return;

        int showItems   = qMin(items.size(), count);
        qreal width     = q->geometry().width();
        sizer->init(showItems);

        QRectF newGeometry = q->geometry();

        int i = 0;

        foreach (Plasma::Widget * item, items) {
            if (items.size() - showItems > i++) {
                if (animator) {
                    animator->setCurrentState(item, Plasma::LayoutAnimator::RemovedState);
                } else {
                    item->setVisible(false);
                }
            } else {
                qreal itemWidth = sizer->size() * width;
                newGeometry.setWidth(itemWidth);
                if (animator) {
                    animator->setGeometry(item, newGeometry);
                    if (!item->isVisible()) {
                        item->setVisible(true);
                        animator->setCurrentState(item, Plasma::LayoutAnimator::InsertedState);
                    } else {
                        animator->setCurrentState(item, Plasma::LayoutAnimator::StandardState);
                    }
                } else {
                    item->setGeometry(newGeometry);
                    if (!item->isVisible()) {
                        item->setVisible(true);
                    }
                }
                newGeometry.moveLeft(newGeometry.left() + itemWidth);
            }
        }
        q->startAnimation();
    }

    void push(Plasma::Widget * widget)
    {
        if (!widget || items.contains(widget)) {
            return;
        }

        widget->setManagingLayout(q);

        //TODO: Uncomment : widget->hide();
        items.append(widget);

        if (animator)  {
            animator->setCurrentState(widget, Plasma::LayoutAnimator::InsertedState);
        }

        q->updateGeometry();
    }

    Plasma::Widget * pop()
    {
        Plasma::Widget * widget = items.takeLast();
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

QSizeF ColumnLayout::sizeHint() const
{
    return QSizeF();
}

void ColumnLayout::relayout()
{
    d->relayout(Private::Resize);
}

void ColumnLayout::push(Plasma::Widget * widget)
{
    d->push(widget);
}

Plasma::Widget * ColumnLayout::pop()
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

ColumnLayout::ColumnLayout(LayoutItem * parent)
    : Plasma::Layout(parent), d(new Private(this))
{

}

ColumnLayout::~ColumnLayout()
{
    delete d;
}

void ColumnLayout::insertItem(int index, Plasma::LayoutItem * item)
{
}

void ColumnLayout::addItem(Plasma::LayoutItem * item)
{
}

void ColumnLayout::removeItem(Plasma::LayoutItem * item)
{
}

int ColumnLayout::indexOf(Plasma::LayoutItem * item) const
{
    return d->items.indexOf((Plasma::Widget *) item);
}

Plasma::LayoutItem * ColumnLayout::itemAt(int i) const
{
    return d->items.at(i);
}

Plasma::LayoutItem * ColumnLayout::takeAt(int i)
{
    return NULL;
}

Qt::Orientations ColumnLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

int ColumnLayout::count() const
{
    return d->items.size();
}

void ColumnLayout::setAnimator(Plasma::LayoutAnimator * animator)
{
    Plasma::Layout::setAnimator(animator);
    d->animator = animator;

    if (animator) {
        d->connect(
                animator, SIGNAL(stateChanged(Plasma::LayoutItem *,
                        Plasma::LayoutAnimator::State, Plasma::LayoutAnimator::State)),
                d, SLOT(stateChanged(Plasma::LayoutItem *,
                        Plasma::LayoutAnimator::State, Plasma::LayoutAnimator::State))
               );
    }

}

void ColumnLayout::releaseManagedItems()
{
    foreach (LayoutItem* item, d->items) {
        item->unsetManagingLayout(this);
    }
}

} // namespace Lancelot

#include "ColumnLayout.moc"

