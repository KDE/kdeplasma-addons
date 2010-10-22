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

#ifndef LANCELOT_CUSTOM_LIST_VIEW_H
#define LANCELOT_CUSTOM_LIST_VIEW_H

#include <KIcon>

#include <lancelot/lancelot_export.h>
#include <lancelot/widgets/ExtenderButton.h>
#include <lancelot/widgets/ScrollPane.h>
#include <lancelot/widgets/Widget.h>

namespace Lancelot
{

/**
 * All classes that are going to be used in the CustomList
 * must subclass this and QGraphicsWidget.
 */
class LANCELOT_EXPORT CustomListItem {
public:
    CustomListItem();
    virtual ~CustomListItem();

    virtual void setSelected(bool selected = true) = 0;
    virtual bool isSelected() const = 0;
};

/**
 * Interface that manages the list model and serves as an interface
 * between the actual data model and CustomList.
 * Subclasses are responsible for creating and destroying list
 * items. All items must subclass QGraphicsWidget and implement
 * the CustomListItem interface.
 */
class LANCELOT_EXPORT CustomListItemFactory: public QObject {
    Q_OBJECT
public:
    CustomListItemFactory();
    virtual ~CustomListItemFactory();

    /**
     * @returns number of items
     */
    virtual int itemCount() const = 0;

    /**
     * @returns an item for the specified index
     * @param index index
     */
    virtual CustomListItem * itemForIndex(int index) = 0;

    /**
     * @returns the height hint for the specified item
     * @param index index of the item
     * @param which which hint to return
     */
    virtual int itemHeight(int index, Qt::SizeHint which) const = 0;

Q_SIGNALS:
    /**
     * This signal is emitted when the model is updated and the update
     * is too complex to explain using itemInserted, itemDeleted and
     * itemAltered methods
     */
    void updated();

    /**
     * This signal is emitted when an item is inserted into the model
     * @param index place where the new item is inserted
     */
    void itemInserted(int index);

    /**
     * This signal is emitted when an item is deleted from the model
     * @param index index of the deleted item
     */
    void itemDeleted(int index);

    /**
     * This signal is emitted when an item is altered
     * @param index index of the altered item
     */
    void itemAltered(int index);
};

/**
 * Class that does the layouting of items in the list.
 * The list implements the Scrollable interface. Supports
 * resizing items to best fit scroll pane viewport.
 * It doesn't scroll by itself.
 */
class LANCELOT_EXPORT CustomList: public QGraphicsWidget, public Scrollable {
    Q_OBJECT
public:
    CustomList(QGraphicsItem * parent = NULL);
    explicit CustomList(CustomListItemFactory * factory,
            QGraphicsItem * parent = NULL);

    virtual ~CustomList();

    void setMargin(Plasma::MarginEdge margin, qreal value);
    qreal margin(Plasma::MarginEdge margin);

    void setItemFactory(CustomListItemFactory * factory);
    CustomListItemFactory * itemFactory() const;

    int itemAtPosition(int y) const;

    L_Override QSizeF sizeFor(QSizeF viewportSize) const;
    L_Override void viewportChanged(QRectF viewport);
    L_Override qreal scrollUnit(Qt::Orientation direction) const;

Q_SIGNALS:
    void sizeChanged();

protected Q_SLOTS:
    void factoryItemInserted(int position);
    void factoryItemDeleted(int position);
    void factoryItemAltered(int position);
    void factoryUpdated();

private:
    class Private;
    Private * const d;

    friend class CustomListView;
};

/**
 * Wrapper around the CustomList which implements the actual
 * scrolling.
 */
class LANCELOT_EXPORT CustomListView: public Lancelot::ScrollPane {
    Q_OBJECT
public:
    CustomListView(QGraphicsItem * parent = NULL);
    explicit CustomListView(CustomListItemFactory * factory,
            QGraphicsItem * parent = NULL);

    virtual ~CustomListView();

    CustomList * list() const;

protected:
    L_Override QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_CUSTOM_LIST_VIEW_H */

