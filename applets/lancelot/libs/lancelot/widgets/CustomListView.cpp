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
#include <lancelot/models/ActionListViewModels.h>

namespace Lancelot {

//> CustomListItem interface
CustomListItem::CustomListItem()
{
}

CustomListItem::~CustomListItem()
{
}
//<

//> CustomListItemFactory
CustomListItemFactory::CustomListItemFactory()
{
}

CustomListItemFactory::~CustomListItemFactory()
{
}
//<

//> CustomList
class CustomList::Private {
public:
    Private(CustomList * parent = NULL,
            CustomListItemFactory * f = NULL,
            AbstractListModel * m = NULL)
        : q(parent), factory(f), model(m), scale(-1)
    {
        recreateItems();
    }

    #define returnIfModelNotSet if (!model || !factory) return

    QGraphicsWidget * itemForIndex(int position) //>
    {
        QGraphicsWidget * item = dynamic_cast < QGraphicsWidget * >
            (factory->itemForIndex(position));

        if (item) {
            item->setParentItem(q);
            items.insert(position, item);
        }
        return item;
    } //<

    void recreateItems() //>
    {
        returnIfModelNotSet;

        freeAllItems();
        for (int i = 0; i < model->size(); i++) {
            itemForIndex(i);
        }

        positionItems();
    } //<

    void freeAllItems() //>
    {
        returnIfModelNotSet;

        factory->freeAllItems();
        items.clear();
    } //<

    void positionItems(int startPosition = 0) //>
    {
        if (startPosition < 0 || startPosition > items.size()) {
            return;
        }

        qreal top(0);

        if (startPosition != 0) {
            top = items.at(startPosition - 1)->geometry().bottom();
        }

        for (int i = startPosition; i < items.size(); i++) {
            int height = (int) (
                    (scale > 0) ?
                        (items.at(i)->maximumHeight() * scale) :
                        (items.at(i)->preferredHeight())
                    );

            items.at(i)->setGeometry(
                0, top,
                viewport.width(), height
                );
            top += height;
        }

        if (items.size() > 0) {
           q->resize(viewport.width(),
                    items.last()->geometry().bottom());
        } else {
            q->resize(0, 0);
        }

        updateSizeInfo();
    } //<

    void updateWidth() //>
    {
        foreach (QGraphicsWidget * item, items) {
            item->resize(
                    viewport.width(),
                    item->size().height());
        }
    } //<

    void insertItem(int position) //>
    {
        returnIfModelNotSet;
        itemForIndex(position);
        positionItems(position);
    } //<

    void removeItem(int position) //>
    {
        returnIfModelNotSet;

        if (position < 0 || position >= items.size()) {
            return;
        }

        factory->freeItem(position);

        positionItems(position);
    } //<

    void updateItem(int position) //>
    {
        positionItems(position);
    } //<

    void updateSizeInfo() //>
    {
        sizes[Qt::MinimumSize]   = 0;
        sizes[Qt::PreferredSize] = 0;
        sizes[Qt::MaximumSize]   = 0;

        if (!model) {
            return;
        }

        for (int i = 0; i < model->size(); i++) {
            sizes[Qt::MinimumSize] +=
                factory->itemHeight(i, Qt::MinimumSize);
            sizes[Qt::PreferredSize] +=
                factory->itemHeight(i, Qt::PreferredSize);
            sizes[Qt::MaximumSize] +=
                factory->itemHeight(i, Qt::MaximumSize);
            kDebug() << "scale" << factory->itemHeight(i, Qt::MinimumSize)
                                << factory->itemHeight(i, Qt::MaximumSize)
                                << factory->itemHeight(i, Qt::PreferredSize);
        }
        kDebug() << "scale" << sizes[Qt::MinimumSize]
                            << sizes[Qt::MaximumSize]
                            << sizes[Qt::PreferredSize];

        if (sizes[Qt::MinimumSize] > viewport.height()) {
            scale = -1;
        } else {
            if (sizes[Qt::MaximumSize] < viewport.height()) {
                scale = 1;
            } else {
                scale = viewport.height() /
                    (qreal) sizes[Qt::MaximumSize];
            }
        }
        kDebug() << "scale" << scale <<
                viewport.height() <<
                sizes[Qt::MaximumSize] <<
                ((ActionListViewModel *)model)->title(0);

        if (q && q->scrollPane()) {
            q->scrollPane()->scrollableWidgetSizeUpdated();
        }
    } //<

    void viewportOriginUpdated() //>
    {
        QTransform transform;
        foreach (QGraphicsWidget * item, items) {
            QRectF itemGeometry = item->geometry();
            if (viewport.intersects(itemGeometry)) {
                item->show();
                transform.reset();
                if (!viewport.contains(itemGeometry)) {
                    QRectF clip = viewport.intersect(itemGeometry);
                    transform.translate(0, clip.top() -
                            itemGeometry.top());
                    transform.scale(1, clip.height() /
                            itemGeometry.height());
                }
                item->setTransform(transform);
            } else {
                item->hide();
            }
        }
    } //<

    void viewportSizeUpdated() //>
    {
        positionItems();
    } //<

    void calculateSizes() //>
    {
        sizes[Qt::MinimumSize]   = 0;
        sizes[Qt::PreferredSize] = 0;
        sizes[Qt::MaximumSize]   = 0;

        if (!model) {
            return;
        }

        for (int i = 0; i < model->size(); i++) {
            sizes[Qt::MinimumSize] +=
                factory->itemHeight(i, Qt::MinimumSize);
            sizes[Qt::PreferredSize] +=
                factory->itemHeight(i, Qt::PreferredSize);
            sizes[Qt::MaximumSize] +=
                factory->itemHeight(i, Qt::MinimumSize);
        }
    } //<

    //> Variable Declarations
    CustomList * q;

    CustomListItemFactory * factory;
    AbstractListModel * model;

    QList < QGraphicsWidget * > items;
    QMap < Qt::SizeHint, int > sizes;

    QRectF viewport;
    qreal scale;
    //<
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

void CustomList::setModel(AbstractListModel * m) //>
{
    if (d->model) {
        disconnect(d->model, NULL, this, NULL);
    }
    d->model = m;

    connect(m, SIGNAL(itemInserted(int)),
            this, SLOT(modelItemInserted(int)));
    connect(m, SIGNAL(itemDeleted(int)),
            this, SLOT(modelItemDeleted(int)));
    connect(m, SIGNAL(itemAltered(int)),
            this, SLOT(modelItemAltered(int)));
    connect(m, SIGNAL(updated()),
            this, SLOT(modelUpdated()));

    d->recreateItems();
} //<

AbstractListModel * CustomList::model() const
{
    return d->model;
}

QSizeF CustomList::fullSize() const //>
{
    d->updateSizeInfo();

    /*if (d->scale > 0) {
        if (d->items.isEmpty()) {
            return QSizeF(0, 0);
        } else {
            return QSizeF(0, d->items.last()->geometry().bottom()); // d->scale * d->sizes[Qt::MaximumSize]);
        }
    } else {
        return QSizeF(0, d->sizes[Qt::PreferredSize]);
    }*/
    return size();
} //<

void CustomList::viewportChanged(QRectF viewport) //>
{
    if (d->viewport.size() != viewport.size()) {
        d->viewport = viewport;
        resize(d->viewport.width(), fullSize().height());
        d->viewportSizeUpdated();
    }
    if (d->viewport.topLeft() != viewport.topLeft()) {
        d->viewport = viewport;
        setPos(- d->viewport.topLeft());
        d->viewportOriginUpdated();
    }
} //<

qreal CustomList::scrollUnit(Qt::Orientation direction)
{
    return 20;
}

void CustomList::modelItemInserted(int position)
{
    d->insertItem(position);
}

void CustomList::modelItemDeleted(int position)
{
    d->removeItem(position);
}

void CustomList::modelItemAltered(int position)
{
    d->updateItem(position);
}

void CustomList::modelUpdated()
{
    d->recreateItems();
}
//<

//> CustomListView
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

//<

} // namespace Lancelot


