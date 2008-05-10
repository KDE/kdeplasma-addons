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

#include "CardLayout.h"
#include <KDebug>
#include <plasma/plasma.h>

namespace Lancelot
{

class CardLayout::Private {
public:
    Private(CardLayout * parent)
      : shown(NULL), q(parent)
    {
    }

    void relayout()
    {
        QRectF g = q->geometry();
        // g.setTopLeft(g.topLeft() + QPointF(margin(Plasma::TopMargin), margin(Plasma::LeftMargin)));
        // g.setBottomRight(g.bottomRight() - QPointF(margin(Plasma::RightMargin), margin(Plasma::BottomMargin)));

        foreach (QGraphicsLayoutItem * l, items) {
            l->setGeometry(g);
        }

        foreach (QGraphicsWidget * l, widgets) {
            l->show();
            l->setGeometry(g);
            if (shown != l) l->hide();
        }
    }

    void removeItem(QGraphicsLayoutItem * item)
    {
        items.removeAll(item);
        QMutableMapIterator<QString, QGraphicsWidget *> i(widgets);
        while (i.hasNext()) {
            i.next();
            if ((QGraphicsLayoutItem *)i.value() == item) {
                i.remove();
            }
        }
    }

    void removeItem(const QString & id)
    {
        widgets.remove(id);
    }

    QList < QGraphicsLayoutItem * > items;
    QMap < QString, QGraphicsWidget * > widgets;
    QGraphicsWidget * shown;
    CardLayout * q;
};

CardLayout::CardLayout(QGraphicsLayoutItem * parent)
  : QGraphicsLayout(parent), d(new Private(this))
{
}

CardLayout::~CardLayout()
{
    delete d;
}

QSizeF CardLayout::sizeHint(Qt::SizeHint which,
            const QSizeF & constraint) const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    foreach (QGraphicsLayoutItem * l, d->items) {
        hintHeight = qMax(l->preferredSize().height(), hintHeight);
        hintWidth  = qMax(l->preferredSize().width(), hintWidth);
    }

    foreach (QGraphicsWidget * l, d->widgets) {
        hintHeight = qMax(l->preferredSize().height(), hintHeight);
        hintWidth  = qMax(l->preferredSize().width(), hintWidth);
    }

    return QSizeF(hintWidth, hintHeight);
}

void CardLayout::addItem(QGraphicsLayoutItem * item)
{
    if (!d->items.contains(item)) {
        d->items.append(item);
    }
}

void CardLayout::addItem(QGraphicsWidget * widget, const QString & id)
{
    if (widget) {
        d->widgets[id] = widget;
        // widget->hide(); // BUGS in QT
    } else {
        d->removeItem(id);
    }
}

int CardLayout::count() const
{
    return d->items.size() + d->widgets.size();
}

QGraphicsLayoutItem * CardLayout::itemAt(int i) const
{
    if (i < d->items.size()) {
        return d->items[i];
    } else {
        i -= d->items.size();
        foreach (QGraphicsWidget * l, d->widgets) {
            if (0 == i--) {
                return (QGraphicsLayoutItem *)l;
            }
        }
    }
    return NULL;
}

void CardLayout::removeAt(int i)
{
    d->removeItem(itemAt(i));
}

void CardLayout::show(const QString & id)
{
    if (!d->widgets.contains(id)) return;
    if (d->shown == d->widgets[id]) return;
    if (d->shown) {
        d->shown->hide();
    }
    d->shown = d->widgets[id];
    d->shown->setGeometry(geometry());
    d->shown->show();
    kDebug() << "Layout geom" << geometry() << "child" << d->shown->geometry();
}

void CardLayout::hideAll()
{
    if (!d->shown) return;
    d->shown->hide();
    d->shown = NULL;
}

void CardLayout::setGeometry(const QRectF & rect)
{
    QGraphicsLayout::setGeometry(rect);
    d->relayout();
}

} // namespace Lancelot

