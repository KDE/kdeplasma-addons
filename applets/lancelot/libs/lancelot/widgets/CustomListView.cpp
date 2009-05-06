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
#include <KIcon>

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
    Private(CustomList * parent = NULL, CustomListItemFactory * f = NULL) //>
        : q(parent), factory(f), scale(-1), sizeUpdate(false)
    {
        margins[Plasma::TopMargin] = 0;
        margins[Plasma::LeftMargin] = 0;
        margins[Plasma::RightMargin] = 0;
        margins[Plasma::BottomMargin] = 0;
    } //<

    #define returnIfFactoryNotSet if (!factory) return

    QGraphicsWidget * itemForIndex(int position) //>
    {
        QGraphicsWidget * item = dynamic_cast < QGraphicsWidget * >
            (factory->itemForIndex(position));

        if (item) {
            item->setParentItem(q);
        }
        return item;
    } //<

    void positionItems(int startPosition = 0) //>
    {
        returnIfFactoryNotSet;

        if (startPosition < 0 || startPosition > factory->itemCount()) {
            return;
        }

        qreal top(margins[Plasma::TopMargin]);

        if (startPosition != 0) {
            top = itemForIndex(startPosition - 1)->geometry().bottom();
        }

        QGraphicsWidget * item;
        for (int i = startPosition; i < factory->itemCount(); i++) {
            item = itemForIndex(i);

            int height = (int) (
                    (scale > 0) ?
                        (item->maximumHeight() * scale) :
                        (item->preferredHeight())
                    );

            item->setGeometry(
                margins[Plasma::LeftMargin], top,
                viewport.width() - margins[Plasma::LeftMargin]
                                 - margins[Plasma::RightMargin],
                height
                );
            top += height;
        }

        /*
        if (factory->itemCount() > 0) {
            q->resize(viewport.width(),
                    item->geometry().bottom() + margins[Plasma::BottomMargin]);
        } else {
            q->resize(0, 0);
        }
        */

        viewportOriginUpdated();
    } //<

    void updateSizeInfo() //>
    {
        QMap < Qt::SizeHint, int > osizes = sizes;

        sizes[Qt::MinimumSize]   = 0;
        sizes[Qt::PreferredSize] = 0;
        sizes[Qt::MaximumSize]   = 0;

        returnIfFactoryNotSet;

        for (int i = 0; i < factory->itemCount(); i++) {
            sizes[Qt::MinimumSize] +=
                factory->itemHeight(i, Qt::MinimumSize);
            sizes[Qt::PreferredSize] +=
                factory->itemHeight(i, Qt::PreferredSize);
            sizes[Qt::MaximumSize] +=
                factory->itemHeight(i, Qt::MaximumSize);
        }

        if (q && q->scrollPane() && osizes != sizes) {
            sizeUpdate = true;
            q->scrollPane()->scrollableWidgetSizeUpdateNeeded();
            sizeUpdate = false;
        }
    } //<

    void viewportOriginUpdated() //>
    {
        returnIfFactoryNotSet;

        // Due to strange Qt behaviour
        // which shows items when show() is
        // invoked although the parent is hidden
        // if (!q->isVisible()) {
        //    return;
        // }

        QTransform transform;
        for (int i = 0; i < factory->itemCount(); i++) {
            QGraphicsWidget * item = itemForIndex(i);
            QRectF itemGeometry = item->geometry();
            if (viewport.intersects(itemGeometry)) {
                //if (q->isVisible()) {
                    item->show();
                //}
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

    //> Variable Declarations
    CustomList * q;
    CustomListItemFactory * factory;
    QMap < Qt::SizeHint, int > sizes;
    QRectF viewport;
    qreal scale;
    QMap < Plasma::MarginEdge, qreal > margins;
    bool sizeUpdate : 1;
    //<
};

CustomList::CustomList(QGraphicsItem * parent) //>
    : QGraphicsWidget(parent), d(new Private(this))
{
} //<

CustomList::CustomList(CustomListItemFactory * factory, QGraphicsItem * parent) //>
    : QGraphicsWidget(parent), d(new Private(this))
{
    setItemFactory(factory);
} //<

CustomList::~CustomList() //>
{
    delete d;
} //<

void CustomList::setItemFactory(CustomListItemFactory * f) //>
{
    // Releasing the old factory
    if (d->factory) {
        disconnect(d->factory, NULL, this, NULL);
    }

    if (!f) {
        return;
    }

    d->factory = f;

    connect(f, SIGNAL(itemInserted(int)),
            this, SLOT(factoryItemInserted(int)));
    connect(f, SIGNAL(itemDeleted(int)),
            this, SLOT(factoryItemDeleted(int)));
    connect(f, SIGNAL(itemAltered(int)),
            this, SLOT(factoryItemAltered(int)));
    connect(f, SIGNAL(updated()),
            this, SLOT(factoryUpdated()));

    d->updateSizeInfo();
} //<

CustomListItemFactory * CustomList::itemFactory() const //>
{
    return d->factory;
} //<

QSizeF CustomList::sizeFor(QSizeF viewportSize) const //>
{
    QSizeF result = viewportSize;
    if (result.height() < d->sizes[Qt::MinimumSize]) {
        result.setHeight(d->sizes[Qt::PreferredSize]);
    } else if (result.height() > d->sizes[Qt::MaximumSize]) {
        result.setHeight(d->sizes[Qt::MaximumSize]);
    }
    return result;
} //<

void CustomList::viewportChanged(QRectF viewport) //>
{
    if (d->sizeUpdate || d->viewport.size() != viewport.size()) {
        if (d->sizes[Qt::MinimumSize] > viewport.height()) {
            d->scale = -1;
        } else {
            if (d->sizes[Qt::MaximumSize] < viewport.height()) {
                d->scale = 1;
            } else {
                d->scale = viewport.height() /
                    (qreal) d->sizes[Qt::MaximumSize];
            }
        }

        d->viewport.setSize(viewport.size());
        resize(d->viewport.width(), sizeFor(viewport.size()).height());
        d->positionItems();
        d->viewportOriginUpdated();
    } //else if (d->viewport.topLeft() != viewport.topLeft()) {
        d->viewport = viewport;
        d->viewportOriginUpdated();
        setPos(- d->viewport.topLeft());
    ///}

    d->viewport = viewport;
} //<

qreal CustomList::scrollUnit(Qt::Orientation direction) const //>
{
    Q_UNUSED(direction);
    return 20;
} //<

void CustomList::factoryItemInserted(int position) //>
{
    Q_UNUSED(position);
    d->updateSizeInfo();
} //<

void CustomList::factoryItemDeleted(int position) //>
{
    Q_UNUSED(position);
    d->updateSizeInfo();
} //<

void CustomList::factoryItemAltered(int position) //>
{
    Q_UNUSED(position);
    d->updateSizeInfo();
} //<

void CustomList::factoryUpdated() //>
{
    d->updateSizeInfo();
} //<

void CustomList::setMargin(Plasma::MarginEdge margin, qreal value) //>
{
    d->margins[margin] = value;
    d->updateSizeInfo();
} //<

qreal CustomList::margin(Plasma::MarginEdge margin) //>
{
    return d->margins[margin];
} //<

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
    clearFlag(ClipScrollable);
    L_WIDGET_SET_INITIALIZED;
}

CustomListView::CustomListView(CustomListItemFactory * factory,
        QGraphicsItem * parent)
    : ScrollPane(parent),
      d(new Private(new CustomList(factory, this), this))
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


