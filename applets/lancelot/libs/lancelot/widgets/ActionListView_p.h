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

#ifndef LANCELOT_ACTION_LIST_VIEW_PH
#define LANCELOT_ACTION_LIST_VIEW_PH

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>

#include <QIcon>

#include <lancelot/widgets/CustomListView.h>
#include <lancelot/widgets/ActionListView.h>
#include <lancelot/models/ActionListModel.h>

namespace Lancelot
{

//> ActionListViewItem
class ActionListViewItemFactory;

class ActionListViewItem: public ExtenderButton, public CustomListItem {
    Q_OBJECT
public:
    ActionListViewItem(ActionListViewItemFactory * factory);
    ~ActionListViewItem();

    L_Override void setSelected(bool selected = true);
    L_Override bool isSelected() const;

    L_Override void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
    L_Override void mousePressEvent(QGraphicsSceneMouseEvent * event);
    L_Override void mouseMoveEvent(QGraphicsSceneMouseEvent * event);

    L_Override void paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
        QWidget * widget);

private:
    QPointF m_mousePos;
    bool m_inSetSelected;
    ActionListViewItemFactory * m_factory;

private Q_SLOTS:
    void select();
    void deselect();
};
//<

//> ActionListViewItemFactory
class ActionListViewItemFactory: public CustomListItemFactory {
    Q_OBJECT
public:
    ActionListViewItemFactory(ActionListModel * model, ActionListView * view, Instance * instance);
    ~ActionListViewItemFactory();

    L_Override CustomListItem * itemForIndex(int index);
    L_Override CustomListItem * itemForIndex(int index,
            bool reload);
    L_Override int itemCount() const;
    L_Override int itemHeight(int index, Qt::SizeHint which) const;

    void setModel(ActionListModel * model);
    ActionListModel * model() const;

    void setExtenderPosition(ExtenderPosition position);
    ExtenderPosition extenderPosition() const;

    void itemContext(ActionListViewItem * sender,
            bool mouseCoordinate = true);
    void itemDrag(ActionListViewItem * sender, QWidget * widget);

    void activateSelectedItem();
    void contextForSelectedItem();
    void selectRelItem(int rel);
    void setSelectedItem(ActionListViewItem * item, bool selected = true);
    void activate(int index);

    void setItemExtender(int index);

    void setItemsGroup(WidgetGroup * group = NULL);
    WidgetGroup * itemsGroup() const;

    void setCategoriesGroup(WidgetGroup * group = NULL);
    WidgetGroup * categoriesGroup() const;

    void clearSelection();
    void updateExtenderPosition();

    void setItemHeight(int height, Qt::SizeHint which);
    void setCategoryHeight(int height, Qt::SizeHint which);
    void setItemIconSize(QSize size);
    void setCategoryIconSize(QSize size);

private:
    void reload();

private Q_SLOTS:
    void modelUpdated();
    void modelItemInserted(int index);
    void modelItemDeleted(int index);
    void modelItemAltered(int index);
    void itemActivated();

Q_SIGNALS:
    void activated(int index);

private:
    ActionListModel * m_model;
    QList < ActionListViewItem * > m_items;
    ExtenderPosition m_extenderPosition;

    WidgetGroup * m_itemsGroup;
    WidgetGroup * m_categoriesGroup;
    Instance * m_instance;
    ActionListView * m_view;

    bool m_categoriesActivable;

    QMap < Qt::SizeHint, int > m_itemHeight;
    QMap < Qt::SizeHint, int > m_categoryHeight;
    QSize m_itemIconSize;
    QSize m_categoryIconSize;

    ActionListViewItem * m_selectedItem;
    friend class ActionListViewItem;
    friend class ActionListView;
};
//<

//> ActionListView
class ActionListView::Private {
public:
    Private();
    ~Private();

    ActionListViewItemFactory * itemFactory;
    bool showsExtendersOutside : 1;
};
//<

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW_PH */

