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

#ifndef LANCELOT_CUSTOM_LIST_VIEW_H_
#define LANCELOT_CUSTOM_LIST_VIEW_H_

#include <lancelot/lancelot_export.h>

#include <QtGui>
#include <QtCore>
#include <KDebug>
#include <KIcon>

#include <lancelot/widgets/Widget.h>
#include <lancelot/widgets/ExtenderButton.h>
#include <lancelot/widgets/ScrollPane.h>

#include <lancelot/models/CustomListModels.h>

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
class LANCELOT_EXPORT CustomListItemFactory {
public:
    CustomListItemFactory();
    virtual ~CustomListItemFactory();

    virtual CustomListItem * itemForIndex(int index) = 0;
    virtual int itemHeight(int index, Qt::SizeHint which) const = 0;

    virtual void freeItem(CustomListItem * item) = 0;
    virtual void freeAllItems() = 0;
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
    CustomList(CustomListItemFactory * factory,
            AbstractListModel * model,
            QGraphicsItem * parent = NULL);

    virtual ~CustomList();

    void setItemFactory(CustomListItemFactory * factory);
    CustomListItemFactory * itemFactory() const;

    void setModel(AbstractListModel * model);
    AbstractListModel * model() const;

    L_Override virtual QSizeF fullSize() const;
    L_Override virtual void viewportChanged(QRectF viewport);
    L_Override virtual qreal scrollUnit(Qt::Orientation direction);

protected Q_SLOTS:
    void modelItemInserted(int position);
    void modelItemRemoved(int position);
    void modelItemAltered(int position);
    void modelUpdated();

private:
    class Private;
    Private * const d;
};

/**
 * Wrapper around the CustomList which implements the actual
 * scrolling.
 */
class LANCELOT_EXPORT CustomListView: public ScrollPane {
    Q_OBJECT
public:
    CustomListView(QGraphicsItem * parent = NULL);
    CustomListView(CustomList * list, QGraphicsItem * parent = NULL);

    virtual ~CustomListView();

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_CUSTOM_LIST_VIEW_H_ */

