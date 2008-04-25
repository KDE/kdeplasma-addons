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

namespace Lancelot
{

class CardLayout::Private {
public:
    Private()
      : shown(NULL)
    {
    }

    QList < Plasma::LayoutItem * > items;
    QMap < QString, Plasma::Widget * > widgets;
    Plasma::Widget * shown;
};

CardLayout::CardLayout(LayoutItem * parent)
  : Plasma::Layout(parent), d(new Private())
{
}

CardLayout::~CardLayout()
{
    delete d;
}

Qt::Orientations CardLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void CardLayout::relayout()
{
    QRectF g = geometry();
    g.setTopLeft(g.topLeft() + QPointF(margin(Plasma::TopMargin), margin(Plasma::LeftMargin)));
    g.setBottomRight(g.bottomRight() - QPointF(margin(Plasma::RightMargin), margin(Plasma::BottomMargin)));

    foreach (Plasma::LayoutItem * l, d->items) {
        l->setGeometry(g);
    }

    foreach (Plasma::Widget * l, d->widgets) {
        l->show();
        l->setGeometry(g);
        if (d->shown != l) l->hide();
    }
}

QSizeF CardLayout::sizeHint() const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    foreach (Plasma::LayoutItem * l, d->items) {
        hintHeight = qMax(l->sizeHint().height(), hintHeight);
        hintWidth  = qMax(l->sizeHint().width(), hintWidth);
    }

    foreach (Plasma::Widget * l, d->widgets) {
        hintHeight = qMax(l->sizeHint().height(), hintHeight);
        hintWidth  = qMax(l->sizeHint().width(), hintWidth);
    }

    return QSizeF(hintWidth, hintHeight);
}

void CardLayout::addItem (Plasma::LayoutItem * item) {
    if (!d->items.contains(item)) {
        d->items.append(item);
    }
}

void CardLayout::addItem (Plasma::Widget * widget, const QString & id) {
    if (widget) {
        d->widgets[id] = widget;
        // widget->hide(); // BUGS in QT
    } else {
        removeItem(id);
    }
}

void CardLayout::removeItem (Plasma::LayoutItem * item) {
    d->items.removeAll(item);
    QMutableMapIterator<QString, Plasma::Widget *> i(d->widgets);
    while (i.hasNext()) {
        i.next();
        if ((Plasma::LayoutItem *)i.value() == item) {
            i.remove();
        }
    }
}

void CardLayout::removeItem (const QString & id) {
    d->widgets.remove(id);
}


int CardLayout::count() const {
    return d->items.size() + d->widgets.size();
}

int CardLayout::indexOf(Plasma::LayoutItem * item) const {
    if (d->items.contains(item)) {
        return d->items.indexOf(item);
    } else {
        int i = d->items.size();
        foreach (Plasma::Widget * l, d->widgets) {
            if ((Plasma::LayoutItem *) l == item) {
                return i;
            }
            ++i;
        }
    }
    return -1;
}

Plasma::LayoutItem * CardLayout::itemAt(int i) const {
    if (i < d->items.size()) {
        return d->items[i];
    } else {
        i -= d->items.size();
        foreach (Plasma::Widget * l, d->widgets) {
            if (0 == i--) {
                return (Plasma::LayoutItem *)l;
            }
        }
    }
    return NULL;
}

Plasma::LayoutItem * CardLayout::takeAt(int i) {
    Plasma::LayoutItem * item = itemAt(i);
    removeItem(item);
    return item;
}

void CardLayout::show(const QString & id) {
    if (!d->widgets.contains(id)) return;
    if (d->shown == d->widgets[id]) return;
    if (d->shown) {
        d->shown->hide();
    }
    d->shown = d->widgets[id];
    d->shown->show();
}

void CardLayout::hideAll() {
    if (!d->shown) return;
    d->shown->hide();
    d->shown = NULL;
}

void CardLayout::releaseManagedItems()
{
    // QList < Plasma::LayoutItem * > d->items;
    // QMap < QString, Plasma::Widget * > d->widgets;

    foreach (Plasma::LayoutItem * item, d->items) {
        item->unsetManagingLayout(this);
    }
    foreach (Plasma::LayoutItem * item, d->widgets) {
        item->unsetManagingLayout(this);
    }
}

} // namespace Lancelot

