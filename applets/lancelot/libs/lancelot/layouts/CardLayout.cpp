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
#include <plasma/plasma.h>
#include <QGraphicsScene>

namespace Lancelot
{

class CardLayout::Private {
public:
    Private(CardLayout * parent)
      : shown(NULL), q(parent)
    {
    }

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

    void relayout()
    {
        QRectF g = q->geometry();

        foreach (QGraphicsLayoutItem * l, items) {
            l->setGeometry(g);
        }

        foreach (QGraphicsWidget * l, widgets) {
            _show(l);
            l->setGeometry(g);
            if (shown != l) {
                _hide(l);
            }
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
    QGraphicsItem * parentItem;
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
    QSizeF size = QSizeF(0, 0);

    foreach (QGraphicsLayoutItem * l, d->items) {
        size = size.expandedTo(l->effectiveSizeHint(which, constraint));
    }

    foreach (QGraphicsWidget * l, d->widgets) {
        size = size.expandedTo(l->effectiveSizeHint(which, constraint));
    }

    return size;
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
        d->_hide(widget);
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
        d->_hide(d->shown);
    }
    d->shown = d->widgets[id];
    d->shown->setGeometry(geometry());
    d->_show(d->shown);
}

void CardLayout::hideAll()
{
    if (!d->shown) return;
    d->_hide(d->shown);
    d->shown = NULL;
}

void CardLayout::setGeometry(const QRectF & rect)
{
    QGraphicsLayout::setGeometry(rect);
    d->relayout();
}

} // namespace Lancelot

