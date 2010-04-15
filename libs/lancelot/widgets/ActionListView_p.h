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

#ifndef LANCELOT_ACTION_LIST_VIEW_PH
#define LANCELOT_ACTION_LIST_VIEW_PH

#include <QIcon>

#include <Plasma/SvgWidget>
#include <Plasma/ItemBackground>

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>
#include <lancelot/models/ActionListModel.h>
#include <lancelot/widgets/ActionListView.h>
#include <lancelot/widgets/CustomListView.h>

#include <lancelot/widgets/CustomItemBackground_p.h>

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
    static Plasma::Svg m_categoryTriangle;

private Q_SLOTS:
    void select();
    void deselect();
};
//<

//> ActionListViewItemFactory
class ActionListViewItemFactory: public CustomListItemFactory {
    Q_OBJECT
public:
    ActionListViewItemFactory(ActionListModel * model, ActionListView * view);
    ~ActionListViewItemFactory();

    L_Override CustomListItem * itemForIndex(int index);
    L_Override CustomListItem * itemForIndex(int index,
            bool reload);
    L_Override int itemCount() const;
    L_Override int itemHeight(int index, Qt::SizeHint which) const;

    void setModel(ActionListModel * model);
    ActionListModel * model() const;

    void setExtenderPosition(int position);
    int extenderPosition() const;

    void setDisplayMode(ActionListView::ItemDisplayMode mode);
    ActionListView::ItemDisplayMode displayMode() const;

    void itemContext(ActionListViewItem * sender,
            bool mouseCoordinate = true);
    void itemDrag(ActionListViewItem * sender, QGraphicsSceneMouseEvent * event);

    void activateSelectedItem();
    void contextForSelectedItem();
    void selectRelItem(int rel);
    void setSelectedItem(ActionListViewItem * item, bool selected = true);
    void activate(int index);

    void setItemExtender(int index);

    void setItemsGroup(Group * group = NULL);
    Group * itemsGroup() const;

    void setCategoriesGroup(Group * group = NULL);
    Group * categoriesGroup() const;

    void clearSelection();
    void updateExtenderPosition();

    void setItemHeight(int height, Qt::SizeHint which);
    void setCategoryHeight(int height, Qt::SizeHint which);

    int itemHeight(Qt::SizeHint which) const;
    int categoryHeight(Qt::SizeHint which) const;

    void setItemIconSize(QSize size);
    void setCategoryIconSize(QSize size);

    QSize itemIconSize() const;
    QSize categoryIconSize() const;

private:
    void reload();

private Q_SLOTS:
    void modelUpdated();
    void modelItemInserted(int index);
    void modelItemDeleted(int index);
    void modelItemAltered(int index);
    void itemActivated();
    void itemHovered();

    void updateSelectedBackground(ActionListViewItem * item = NULL);

    qreal preferredWidth() const;

Q_SIGNALS:
    void activated(int index);

private:
    ActionListModel * m_model;
    QList < ActionListViewItem * > m_items;
    int m_extenderPosition;


    Group * m_itemsGroup;
    Group * m_categoriesGroup;
    ActionListView * m_view;

    bool m_categoriesActivable;

    QMap < Qt::SizeHint, int > m_itemHeight;
    QMap < Qt::SizeHint, int > m_categoryHeight;
    QSize m_itemIconSize;
    QSize m_categoryIconSize;

    ActionListViewItem * m_selectedItem;
    ActionListView::ItemDisplayMode m_displayMode;

    CustomItemBackground * m_selectedItemBackground;

    friend class ActionListViewItem;
    friend class ActionListView;
};
//<

//> ActionListView
class ActionListView::Private: public QObject {
    Q_OBJECT
public:
    Private(ActionListView * listView);
    ~Private();

    ActionListViewItemFactory * itemFactory;
    Plasma::SvgWidget * dropIndicator;
    bool showsExtendersOutside : 1;
    ActionListView * const q;

public Q_SLOTS:
    void immutabilityChanged(const Plasma::ImmutabilityType immutable);
    void sizeHintUpdateNeeded();

};
//<

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW_PH */

