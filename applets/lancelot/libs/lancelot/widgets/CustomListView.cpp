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

#include "CustomListView.h"
#include <KDebug>
#include <KIcon>

namespace Lancelot
{

// CustomListItem interface
CustomListItem::CustomListItem()
{
}

CustomListItem::~CustomListItem()
{
}

// CustomListItemFactory
CustomListItemFactory::CustomListItemFactory()
{
}

CustomListItemFactory::~CustomListItemFactory()
{
}

// CustomList
class CustomList::Private {
public:
    Private(CustomList * parent = NULL,
            CustomListItemFactory * f = NULL,
            AbstractListModel * m = NULL)
        : q(parent), factory(f), model(m)
    {
        recreateItems();
    }

    void calculateSizes()
    {
        sizes[Qt::MinimumSize]   = 0;
        sizes[Qt::PreferredSize] = 0;
        sizes[Qt::MaximumSize]   = 0;
        for (int i = 0; i < model->size(); i++) {
            sizes[Qt::MinimumSize] +=
                factory->itemHeight(i, Qt::MinimumSize);
            sizes[Qt::PreferredSize] +=
                factory->itemHeight(i, Qt::PreferredSize);
            sizes[Qt::MaximumSize] +=
                factory->itemHeight(i, Qt::MinimumSize);
        }
    }

    void freeAllItems()
    {
        factory->freeAllItems();
        items.clear();
    }

    void recreateItems()
    {
        kDebug() << "entered" << model << factory;
        if (!model || !factory) {
            return;
        }
        kDebug() << "go! need:" << model->size() << "items";

        freeAllItems();
        qreal top = 0;

        for (int i = 0; i < model->size(); i++) {
            QGraphicsWidget * item =
                dynamic_cast < QGraphicsWidget * >
                (factory->itemForIndex(i));

            if (!item) {
                continue;
            }

            kDebug() << "adding";
            item->setParentItem(q);
            item->setGeometry(0, top,
                    viewportSize.width(), item->preferredHeight()
            );
            kDebug() << "item geometry" << item->geometry();
            top += item->preferredHeight();
        }
    }

    void insertItem(int position)
    {
        if (!model || !factory) {
            return;
        }

        if (position < 0) {
            position = 0;
        }

        QGraphicsWidget * item = dynamic_cast < QGraphicsWidget * >
            (factory->itemForIndex(position));

        if (!item) {
            return;
        }

        qreal top = 0;
        if (position >= items.size()) {
            if (items.size() > 0) {
                top = items.last()->geometry().bottom();
            }
        } else {
            top = items.at(position)->geometry().top();
        }

        item->setParentItem(q);
        item->setGeometry(QRectF(
            QPointF(0, top),
            item->preferredSize()
        ));
        items.insert(position, item);

        top = item->preferredHeight();

        for (int i = position + 1; i < items.size(); i++) {
            items.at(i)->moveBy(0, top);
        }
    }

    void removeItem(int position)
    {
        if (!model || !factory || position >= items.size() || position < 0) {
            return;
        }

        qreal shift = items.at(position)->
            geometry().height();
        factory->freeItem(position);

        for (int i = position; i < items.size(); i++) {
            items.at(i)->moveBy(0, - shift);
        }
    }

    void invalidateItem(int position)
    {
        if (!model || !factory || position + 1 >= items.size() || position < 0) {
            return;
        }

        qreal shift =
            items.at(position + 1)->geometry().top() -
            items.at(position)->geometry().bottom();

        for (int i = position; i < items.size(); i++) {
            items.at(i)->moveBy(0, - shift);
        }
    }

    void updateItemSizes()
    {
        foreach (QGraphicsWidget * item, items) {
            item->resize(viewportSize.width(), item->size().height());
        }
    }

    CustomList * q;
    CustomListItemFactory * factory;
    AbstractListModel * model;
    QList < QGraphicsWidget * > items;
    QMap < Qt::SizeHint, int > sizes;
    QSizeF viewportSize;
    QPointF viewportOrigin;
};

CustomList::CustomList(QGraphicsItem * parent)
    : QGraphicsWidget(parent), d(new Private(this))
{
}

CustomList::CustomList(
        CustomListItemFactory * factory, AbstractListModel * model,
        QGraphicsItem * parent)
    : QGraphicsWidget(parent), d(new Private(this, factory, model))
{
}

CustomList::~CustomList()
{
    delete d;
}

void CustomList::setItemFactory(CustomListItemFactory * factory)
{
    // Releasing the old factory
    if (d->factory) {
        d->freeAllItems();
    }

    d->factory = factory;
    d->recreateItems();
}

CustomListItemFactory * CustomList::itemFactory() const
{
    return d->factory;
}

void CustomList::setModel(AbstractListModel * m)
{
    if (d->model) {
        disconnect(d->model, NULL, this, NULL);
    }
    d->model = m;

    connect(m, SIGNAL(itemInserted(int)),
            this, SLOT(modelItemInserted(int)));
    connect(m, SIGNAL(itemRemoved(int)),
            this, SLOT(modelItemRemoved(int)));
    connect(m, SIGNAL(itemAltered(int)),
            this, SLOT(modelItemAltered(int)));
    connect(m, SIGNAL(updated(int)),
            this, SLOT(modelUpdated(int)));

    d->recreateItems();
}

AbstractListModel * CustomList::model() const
{
    return d->model;
}

QSizeF CustomList::fullSize() const
{
    if (d->viewportSize.height() < d->sizes[Qt::MinimumSize]) {
        return QSizeF(0, d->sizes[Qt::PreferredSize]);
    } else {
        return QSizeF(0, d->viewportSize.height());
    }
}

void CustomList::viewportChanged(QRectF viewport)
{
    if (d->viewportSize != viewport.size()) {
        kDebug() << "size changed" << d->viewportSize <<
            viewport.size();

        d->viewportSize = viewport.size();
        resize(d->viewportSize.width(), fullSize().height());
        d->updateItemSizes();
    }
    if (d->viewportOrigin != viewport.topLeft()) {
        kDebug() << "origin changed" << d->viewportOrigin <<
            viewport.topLeft();
        d->viewportOrigin = viewport.topLeft();
        //setGeometry(QRectF(
        //        - d->viewportOrigin,
        //        geometry().size()
        //    ));
        setPos(- d->viewportOrigin);
    }
}

qreal CustomList::scrollUnit(Qt::Orientation direction)
{
    return 20;
}

void CustomList::modelItemInserted(int position)
{
    d->insertItem(position);
    d->calculateSizes();
}

void CustomList::modelItemRemoved(int position)
{
    d->removeItem(position);
    d->calculateSizes();
}

void CustomList::modelItemAltered(int position)
{
    d->invalidateItem(position);
    d->calculateSizes();
}

void CustomList::modelUpdated()
{
    d->calculateSizes();
}

// CustomListView
class CustomListView::Private {
public:
    Private(CustomList * l, CustomListView * parent)
        : list(l)
    {
        parent->setScrollableWidget(list);
    }

    CustomList * list;
};

CustomListView::CustomListView(QGraphicsItem * parent)
    : ScrollPane(parent), d(new Private(new CustomList(this), this))
{
    L_WIDGET_SET_INITIALIZED;
}

CustomListView::CustomListView(CustomListItemFactory * factory,
        AbstractListModel * model, QGraphicsItem * parent)
    : ScrollPane(parent),
      d(new Private(new CustomList(factory, model, this), this))
{
    L_WIDGET_SET_INITIALIZED;
}

CustomListView::~CustomListView()
{
    delete d;
}

CustomList * CustomListView::list() const
{
    return d->list;
}

} // namespace Lancelot


