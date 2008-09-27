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

#ifndef LANCELOT_ACTION_LIST_VIEW2_PH_
#define LANCELOT_ACTION_LIST_VIEW2_PH_

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>

#include <QtGui>
#include <QtCore>
#include <KDebug>
#include <QIcon>

#include <lancelot/widgets/CustomListView.h>
#include <lancelot/widgets/ActionListView2.h>
#include <lancelot/models/ActionListViewModels.h>

namespace Lancelot
{

//> ActionListView2Item
class ActionListView2ItemFactory;

class ActionListView2Item: public ExtenderButton, public CustomListItem {
    Q_OBJECT;
public:
    ActionListView2Item(ActionListView2ItemFactory * factory);
    ~ActionListView2Item();

    L_Override virtual void setSelected(bool selected = true);
    L_Override virtual bool isSelected() const;

    L_Override virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
    L_Override virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    L_Override virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);

private:
    QPointF m_mousePos;
    ActionListView2ItemFactory * m_factory;

private Q_SLOTS:
    void select();
    void deselect();
};
//<

//> ActionListView2ItemFactory
class ActionListView2ItemFactory: public CustomListItemFactory {
    Q_OBJECT
public:
    ActionListView2ItemFactory(ActionListViewModel * model);
    ~ActionListView2ItemFactory();

    L_Override virtual CustomListItem * itemForIndex(int index);
    L_Override virtual CustomListItem * itemForIndex(int index,
            bool reload);
    L_Override int itemCount() const;
    L_Override virtual int itemHeight(int index, Qt::SizeHint which) const;

    bool categoriesActivable() const;
    void setCategoriesActivable(bool value);

    void setModel(ActionListViewModel * model);
    ActionListViewModel * model();

    void setExtenderPosition(ExtenderPosition position);
    ExtenderPosition extenderPosition() const;

    void itemContext(ActionListView2Item * sender);
    void itemDrag(ActionListView2Item * sender, QWidget * widget);

    void activateSelectedItem();
    void selectRelItem(int rel);
    void activate(int index);

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
    ActionListViewModel * m_model;
    QList < ActionListView2Item * > m_items;
    bool m_categoriesActivable : 1;
    ExtenderPosition m_extenderPosition;

    ActionListView2Item * m_selectedItem;
    friend class ActionListView2Item;
};
//<

//> ActionListView2
class ActionListView2::Private {
public:
    Private();
    ~Private();

    ActionListView2ItemFactory * itemFactory;
};
//<

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW2_PH_ */

