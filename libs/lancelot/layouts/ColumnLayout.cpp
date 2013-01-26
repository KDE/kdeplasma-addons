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

#include "ColumnLayout.h"

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QPropertyAnimation>
#include <QList>
#include <QDebug>
#include <QPointer>

#include <Global.h>
#include <KConfig>
#include <KConfigGroup>

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
    QHash < QGraphicsWidget *, QPointer < QPropertyAnimation > > animators;

    QGraphicsItem * parentItem;
    ColumnLayout::ColumnSizer * sizer;

    int count;
    bool animate;

    enum RelayoutType { Clean, Push, Pop, Resize };

    Private(ColumnLayout * parent)
        : q(parent), parentItem(NULL),
          sizer(new GoldenColumnSizer()), count(2)
    {
        animate = !Global::self()->config("Animation", "disableAnimations", false)
                && Global::self()->config("Animation", "columnLayoutAnimaiton", true);
    }
    ~Private()
    {
        delete sizer;
    }

    QPropertyAnimation * animator(QGraphicsWidget * item, const QByteArray & property)
    {
        QPropertyAnimation * result;

        if (animators.contains(item) && animators[item]) {
            result = animators[item];
            result->stop();
            result->setPropertyName(property);

        } else {
            animators[item] = new QPropertyAnimation(item, property);
            result = animators[item];
        }

        return result;
    }

    void moveItemTo(QGraphicsWidget * item, QRectF newGeometry)
    {
        if (!animate) {
            item->setGeometry(newGeometry);
            item->show();
            item->setOpacity(1);
            return;
        }

        if (newGeometry.width() < 1) {
            hideItem(item);
        }

        if (item->isVisible()) {
            // Moving the item

            if (item->geometry().height() == newGeometry.height()) {
                QPropertyAnimation * animation = animator(item, "geometry");
                animation->setDuration(300);
                animation->setStartValue(item->geometry());
                animation->setEndValue(newGeometry);
                animation->start();

            } else {
                item->setGeometry(newGeometry);

            }

        } else {
            // Showing the item

            item->setGeometry(newGeometry);
            item->setOpacity(0);
            item->show();

            QPropertyAnimation * animation = animator(item, "opacity");
            animation->setDuration(300);
            animation->setStartValue(0);
            animation->setEndValue(1);
            animation->start();

       }
    }

    void hideItem(QGraphicsWidget * item)
    {
        if (!animate) {
            item->hide();
            return;
        }

        if (!item->isVisible()) {
            return;
        }

        item->setOpacity(1);

        QPropertyAnimation * animation = animator(item, "opacity");
        animation->setDuration(300);
        animation->setStartValue(1);
        animation->setEndValue(0);
        animation->start();

        //item->connect(animation, SIGNAL(finished()), SLOT(hide()));
    }

    void relayout(RelayoutType type = Clean)
    {
        Q_UNUSED(type);
        if (items.size() == 0) return;

        int showItems   = qMin(items.size(), count + 1);
        sizer->init(showItems);

        qreal left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);

        QRectF newGeometry = q->geometry().adjusted(left, top, right, bottom);
        qreal width = newGeometry.width();


        int i = 0;

        foreach (QGraphicsWidget * item, items) {
            if (items.size() - showItems > i++) {
                hideItem(item);

            } else {
                int itemWidth = sizer->size() * width;

                newGeometry.setWidth(itemWidth);

                moveItemTo(item, newGeometry);

                newGeometry.moveLeft(newGeometry.left() + itemWidth);
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

        animators.remove(widget);

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

