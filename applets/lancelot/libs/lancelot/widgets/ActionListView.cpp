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

#include "ActionListView.h"
#include "ActionListView_p.h"

#include <QGraphicsSceneMouseEvent>
#include <QApplication>

namespace Lancelot {

//> ActionListViewItem
ActionListViewItem::ActionListViewItem(ActionListViewItemFactory * factory)
    : ExtenderButton(), m_inSetSelected(false), m_factory(factory)
{
    connect(this, SIGNAL(activated()),
            this, SLOT(select()));
    // setPaintBackwardsWhenRTL(true);
    L_WIDGET_SET_INITIALIZED;
}

ActionListViewItem::~ActionListViewItem()
{
}

void ActionListViewItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
        QWidget * widget) {
    if (isSelected() && !isHovered()) {
        if (Plasma::FrameSvg * svg = group()->backgroundSvg()) {
            svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        }
        paintBackground(painter, "active");
        paintForeground(painter);
    } else {
        ExtenderButton::paint(painter, option, widget);
    }
}

void ActionListViewItem::select()
{
    setSelected(true);
}

void ActionListViewItem::deselect()
{
    setSelected(false);
    m_factory->m_selectedItem = NULL;
}

void ActionListViewItem::setSelected(bool selected)
{
    if (m_inSetSelected) return;
    m_inSetSelected = true;

    // setHovered(selected);

    m_factory->setSelectedItem(this, selected);

    if (!selected) {
        hoverLeaveEvent(NULL);
    } else {
        m_factory->m_view->scrollTo(geometry());
        hoverEnterEvent(NULL);
    }

    update();
    m_inSetSelected = false;
}

bool ActionListViewItem::isSelected() const
{
    return m_factory->m_selectedItem == this;
}

void ActionListViewItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
    Q_UNUSED(event);
    m_factory->itemContext(this);
}

void ActionListViewItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_mousePos = event->pos();
    ExtenderButton::mousePressEvent(event);
}

void ActionListViewItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    ExtenderButton::mouseMoveEvent(event);
    if (isDown() && ((m_mousePos - event->pos()).toPoint().manhattanLength() > QApplication::startDragDistance())) {
        setDown(false);
        m_factory->itemDrag(this, event->widget());
    }
}

//<

//> ActionListViewItemFactory
ActionListViewItemFactory::ActionListViewItemFactory(ActionListModel * model, ActionListView * view, Instance * instance) //>
    : m_model(NULL),
      m_extenderPosition(NoExtender),
      m_itemsGroup(NULL), m_categoriesGroup(NULL),
      m_instance(instance), m_view(view),
      m_categoriesActivable(false), m_itemIconSize(32, 32),
      m_categoryIconSize(20, 20), m_selectedItem(NULL)
{
    setItemsGroup(NULL);
    setCategoriesGroup(NULL);
    setModel(model);

    m_categoryHeight[Qt::MinimumSize] = 20;
    m_categoryHeight[Qt::MaximumSize] = 35;
    m_categoryHeight[Qt::PreferredSize] = 27;

    m_itemHeight[Qt::MinimumSize] = 40;
    m_itemHeight[Qt::MaximumSize] = 70;
    m_itemHeight[Qt::PreferredSize] = 55;
} //<

ActionListViewItemFactory::~ActionListViewItemFactory() //>
{
    qDeleteAll(m_items);
    m_items.clear();
} //<

void ActionListViewItemFactory::reload() //>
{
    while (m_items.size() > m_model->size()) {
        ActionListViewItem * item = m_items.takeLast();
        if (m_selectedItem == item) {
            m_selectedItem = NULL;
        }
        item->hide();
        item->deleteLater();
    }

    for (int i = 0; i < m_model->size(); i++) {
        itemForIndex(i, true);
    }

    emit updated();
} //<

CustomListItem * ActionListViewItemFactory::itemForIndex(int index) //>
{
    return itemForIndex(index, false);
} //<

CustomListItem * ActionListViewItemFactory::itemForIndex(int index,
        bool reload) //>
{
    ActionListViewItem * item;
    if (index < m_items.size() && m_items[index]) {
        item = m_items[index];
    } else {
        item = new ActionListViewItem(this);
        item->setGroup((m_model->isCategory(index))
                ? m_categoriesGroup : m_itemsGroup);
        reload = true;
        while (index >= m_items.size()) {
            m_items.append(NULL);
        }
        m_items[index] = item;
        setItemExtender(index);
        connect(item, SIGNAL(activated()),
                this, SLOT(itemActivated()));
    }

    if (reload) {
        item->setTitle(m_model->title(index));
        item->setDescription(m_model->description(index));
        item->setIcon(m_model->icon(index));
        item->setMinimumHeight(itemHeight(index, Qt::MinimumSize));
        item->setPreferredHeight(itemHeight(index, Qt::PreferredSize));
        item->setMaximumHeight(itemHeight(index, Qt::MaximumSize));
        item->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        if (m_model->isCategory(index)) {
            item->setGroup(m_categoriesGroup);
            item->setIconSize(m_categoryIconSize);
        } else {
            item->setGroup(m_itemsGroup);
            item->setIconSize(m_itemIconSize);
        }
    }

    return item;
} //<

void ActionListViewItemFactory::setItemsGroup(WidgetGroup * group) //>
{
    if (group == NULL) {
        group = m_instance->group("ActionListView-Items");
    }

    if (group == m_itemsGroup) return;

    m_itemsGroup = group;

    int i = 0;
    foreach(ActionListViewItem * item, m_items) {
        if (!(m_model->isCategory(i))) {
            item->setGroup(group);
        }
        i++;
    }
}

WidgetGroup * ActionListViewItemFactory::itemsGroup() const //>
{
    return m_itemsGroup;
} //<

void ActionListViewItemFactory::setCategoriesGroup(WidgetGroup * group) //>
{
    if (group == NULL) {
        group = m_instance->group("ActionListView-Categories");
    }

    if (group == m_itemsGroup) return;

    m_categoriesGroup = group;

    int i = 0;
    foreach(ActionListViewItem * item, m_items) {
        if (m_model->isCategory(i)) {
            item->setGroup(group);
            setItemExtender(i);
        }
        i++;
    }
}

WidgetGroup * ActionListViewItemFactory::categoriesGroup() const //>
{
    return m_categoriesGroup;
} //<

void ActionListViewItemFactory::itemActivated() //>
{
    if (!sender()) {
        return;
    }

    activate(m_items.indexOf(
                (Lancelot::ActionListViewItem *)sender()));
} //<

void ActionListViewItemFactory::activate(int index) //>
{
    if (index < 0 || index >= m_model->size()) {
        return;
    }
    m_model->activated(index);
    emit activated(index);
} //<

int ActionListViewItemFactory::itemCount() const //>
{
    if (m_model) {
        return m_model->size();
    } else {
        return 0;
    }
} //<

void ActionListViewItemFactory::setItemHeight(int height, Qt::SizeHint which)
{
    m_itemHeight[which] = height;
    foreach (ActionListViewItem * item, m_items) {
        int index = m_items.indexOf(item);
        item->setMinimumHeight(itemHeight(index, Qt::MinimumSize));
        item->setPreferredHeight(itemHeight(index, Qt::PreferredSize));
        item->setMaximumHeight(itemHeight(index, Qt::MaximumSize));
    }
    emit updated();
}

void ActionListViewItemFactory::setCategoryHeight(int height, Qt::SizeHint which)
{
    m_categoryHeight[which] = height;
    foreach (ActionListViewItem * item, m_items) {
        int index = m_items.indexOf(item);
        item->setMinimumHeight(itemHeight(index, Qt::MinimumSize));
        item->setPreferredHeight(itemHeight(index, Qt::PreferredSize));
        item->setMaximumHeight(itemHeight(index, Qt::MaximumSize));
    }
    emit updated();
}

void ActionListViewItemFactory::setItemIconSize(QSize size)
{
    m_itemIconSize = size;

    int i = 0;
    foreach (ActionListViewItem * item, m_items) {
        if (!m_model->isCategory(i)) {
            item->setIconSize(size);
        }
        i++;
    }
    emit updated();
}

void ActionListViewItemFactory::setCategoryIconSize(QSize size)
{
    m_categoryIconSize = size;

    int i = 0;
    foreach (ActionListViewItem * item, m_items) {
        if (m_model->isCategory(i)) {
            item->setIconSize(size);
        }
        i++;
    }
    emit updated();
}

int ActionListViewItemFactory::itemHeight(int index, Qt::SizeHint which) const //>
{
    if (m_model->isCategory(index)) {
        return m_categoryHeight[which];
    } else {
        return m_itemHeight[which];
    }
} //<

void ActionListViewItemFactory::setModel(ActionListModel * model) //>
{
    ActionListModel * oldmodel = m_model;

    if (m_model) {
        disconnect(m_model, NULL, this, NULL);
    }

    if (!model) {
        return;
    }

    m_model = model;

    connect(model, SIGNAL(itemInserted(int)),
            this, SLOT(modelItemInserted(int)));
    connect(model, SIGNAL(itemDeleted(int)),
            this, SLOT(modelItemDeleted(int)));
    connect(model, SIGNAL(itemAltered(int)),
            this, SLOT(modelItemAltered(int)));
    connect(model, SIGNAL(updated()),
            this, SLOT(modelUpdated()));

    if (oldmodel) {
        modelUpdated();
    }
} //<

ActionListModel * ActionListViewItemFactory::model() const //>
{
    return m_model;
} //<

void ActionListViewItemFactory::modelUpdated() //>
{
    reload();
} //<

void ActionListViewItemFactory::modelItemInserted(int index) //>
{
    if (index < 0 || index > m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        m_items.insert(index, NULL);
        itemForIndex(index, true);
        emit itemInserted(index);
    }
} //<

void ActionListViewItemFactory::modelItemDeleted(int index) //>
{
    if (index < 0 || index >= m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        ActionListViewItem * item = m_items.takeAt(index);
        if (m_selectedItem == item) {
            m_selectedItem = NULL;
        }
        delete item;
        emit itemDeleted(index);
    }
} //<

void ActionListViewItemFactory::modelItemAltered(int index) //>
{
    if (index < 0 || index >= m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        itemForIndex(index, true);
        emit itemAltered(index);
    }
} //<

void ActionListViewItemFactory::setExtenderPosition(ExtenderPosition position) //>
{
    if (position == TopExtender) {
        position = LeftExtender;
    }

    if (position == BottomExtender) {
        position = RightExtender;
    }

    m_extenderPosition = position;
    updateExtenderPosition();
} //<

void ActionListViewItemFactory::updateExtenderPosition() //>
{
    for (int i = 0; i < m_items.count(); i++) {
        setItemExtender(i);
    }
} //<

void ActionListViewItemFactory::setItemExtender(int index) //>
{
    ActionListViewItem * item = m_items.at(index);
    if (m_model->isCategory(index) && !m_categoriesActivable) {
        item->setExtenderPosition(NoExtender);
    } else {
        item->setExtenderPosition(m_extenderPosition);
    }
} //<

ExtenderPosition ActionListViewItemFactory::extenderPosition() const //>
{
    return m_extenderPosition;
} //<

void ActionListViewItemFactory::itemContext(
        ActionListViewItem * sender, bool mouseCoordinate) //>
{
    int index = m_items.indexOf(sender);
    if (index < 0 || index >= m_model->size() ||
            !m_model->hasContextActions(index)) {
        return;
    }

    Lancelot::PopupMenu menu;
    QPoint popupPoint;
    m_model->setContextActions(index, &menu);
    if (!mouseCoordinate) {
        QGraphicsScene * scene = sender->scene();
        if (scene->views().size()) {
            QGraphicsView * view = scene->views().at(0);
            QPointF pos = sender->mapToScene(
                    QPointF(sender->geometry().width() * 0.7, 0));
            popupPoint = view->mapToGlobal(
                view->mapFromScene(pos)
            );
        } else {
            mouseCoordinate = true;
        }
    }

    if (mouseCoordinate) {
        popupPoint = QCursor::pos();
    }

    m_model->contextActivate(index, menu.exec(popupPoint));
} //<

void ActionListViewItemFactory::itemDrag(ActionListViewItem * sender, QWidget * widget) //>
{
    int index = m_items.indexOf(sender);
    if (index < 0 || index >= m_model->size()) {
        return;
    }

    QMimeData * data = m_model->mimeData(index);
    if (data == NULL) {
        return;
    }

    QDrag * drag = new QDrag(widget);
    drag->setMimeData(data);

    Qt::DropActions actions;
    Qt::DropAction defaultAction;
    m_model->setDropActions(index, actions, defaultAction);

    Qt::DropAction dropAction = drag->exec(actions, defaultAction);
    m_model->dataDropped(index, dropAction);

} //<

void ActionListViewItemFactory::activateSelectedItem() //>
{
    if (!m_selectedItem) {
        return;
    }

    activate(m_items.indexOf(m_selectedItem));
} //<

void ActionListViewItemFactory::contextForSelectedItem() //>
{
    if (!m_selectedItem) {
        return;
    }

    itemContext(m_selectedItem, false);
} //<

void ActionListViewItemFactory::clearSelection() //>
{
    if (m_selectedItem) {
        setSelectedItem(m_selectedItem, false);
    }
}

void ActionListViewItemFactory::setSelectedItem(ActionListViewItem * item, bool selected) //>
{
    if (m_selectedItem == item && !selected) {
        if (m_selectedItem) {
            m_selectedItem->setSelected(false);
        }
        m_selectedItem = NULL;
    } else if (m_selectedItem != item && selected) {
        if (m_selectedItem) {
            m_selectedItem->setSelected(false);
        }
        m_selectedItem = item;
        m_selectedItem->setSelected(true);
    }
} //<

void ActionListViewItemFactory::selectRelItem(int rel) //>
{
    int index = -1;
    if (m_selectedItem) {
        index = m_items.indexOf(m_selectedItem);
    }

    if (index == -1) {
        if (rel == 1) {
            index = 0;
        } else {
            index = m_items.count() - 1;
        }
    } else {
        if (rel == 1) {
            index++;
            if (index >= m_items.count()) {
                index = 0;
            }
        } else {
            index--;
            if (index < 0) {
                index = m_items.count() - 1;
            }
        }
    }

    if (!m_categoriesActivable) {
        int oindex = index;
        while (m_model->isCategory(index)) {
            index += rel;
            if (index == oindex) return;

            if (index < 0) {
                index = m_items.count() - 1;
            } else if (index >= m_items.count()) {
                index = 0;
            }
        }
    }

    if (index >= 0 && index < m_items.count()) {
        m_items.at(index)->setSelected();
    }
} //<

//<

//> ActionListView
ActionListView::Private::Private() //>
    : itemFactory(NULL)
{
} //<

ActionListView::Private::~Private() //>
{
    delete itemFactory;
} //<

ActionListView::ActionListView(QGraphicsItem * parent) //>
    : CustomListView(parent), d(new Private())
{
    setModel(NULL);
    setFlag(ScrollPane::HoverShowScrollbars);
    clearFlag(ScrollPane::ClipScrollable);
    setFocusPolicy(Qt::WheelFocus);

    L_WIDGET_SET_INITIALIZED;
} //<

ActionListView::ActionListView(ActionListModel * model, QGraphicsItem * parent) //>
    : CustomListView(parent),
      d(new Private())
{
    setModel(model);
    // d->itemFactory = (ActionListViewItemFactory *) list()->itemFactory();
    connect(
            d->itemFactory, SIGNAL(activated(int)),
            this, SIGNAL(activated(int)));

    setFlag(ScrollPane::HoverShowScrollbars);
    clearFlag(ScrollPane::ClipScrollable);
    setFocusPolicy(Qt::WheelFocus);

    L_WIDGET_SET_INITIALIZED;
} //<

ActionListView::~ActionListView() //>
{
    delete d;
} //<

void ActionListView::setModel(ActionListModel * model) //>
{
    if (!d->itemFactory) {
        d->itemFactory = new ActionListViewItemFactory(model, this, instance());
        list()->setItemFactory(d->itemFactory);
    } else {
        d->itemFactory->setModel(model);
    }
} //<

ActionListModel * ActionListView::model() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }
    return d->itemFactory->model();
} //<

void ActionListView::setItemHeight(int height, Qt::SizeHint which)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setItemHeight(height, which);
}

void ActionListView::setCategoryHeight(int height, Qt::SizeHint which)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setCategoryHeight(height, which);
}

void ActionListView::setItemIconSize(QSize size)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setItemIconSize(size);
}

void ActionListView::setCategoryIconSize(QSize size)
{
    if (!d->itemFactory) {
        return;
    }
    return d->itemFactory->setCategoryIconSize(size);
}

void ActionListView::setShowsExtendersOutside(bool value)
{
    d->showsExtendersOutside = value;
    if (!d->itemFactory) {
        return;
    }
    // relayouting:
    setExtenderPosition(d->itemFactory->extenderPosition());
}

bool ActionListView::showsExtendersOutside() const
{
    return d->showsExtendersOutside;
}


void ActionListView::setExtenderPosition(ExtenderPosition position) //>
{
    if (!d->itemFactory) {
        return;
    }
    d->itemFactory->setExtenderPosition(position);

    if (d->itemFactory->extenderPosition() == LeftExtender) {
        list()->setMargin(Plasma::LeftMargin,
                d->showsExtendersOutside ? 0 : EXTENDER_SIZE);
        list()->setMargin(Plasma::RightMargin, 0);
        setFlip(Plasma::NoFlip);
    } else if (d->itemFactory->extenderPosition() == RightExtender) {
        list()->setMargin(Plasma::LeftMargin, 0);
        list()->setMargin(Plasma::RightMargin,
                d->showsExtendersOutside ? 0 : EXTENDER_SIZE);
        setFlip(Plasma::HorizontalFlip);
    } else {
        list()->setMargin(Plasma::LeftMargin, 0);
        list()->setMargin(Plasma::RightMargin, 0);
        setFlip(Plasma::NoFlip);
    }

} //<

ExtenderPosition ActionListView::extenderPosition() const //>
{
    if (!d->itemFactory) {
        return NoExtender;
    }
    return d->itemFactory->extenderPosition();
} //<

void ActionListView::setItemsGroup(WidgetGroup * group) //>
{
    if (!d->itemFactory) {
        return;
    }

    d->itemFactory->setItemsGroup(group);
} //<

void ActionListView::setItemsGroupByName(const QString & group) //>
{
    setItemsGroup(instance()->group(group));
} //<

WidgetGroup * ActionListView::itemsGroup() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }

    return d->itemFactory->itemsGroup();
} //<

void ActionListView::setCategoriesGroup(WidgetGroup * group) //>
{
    if (!d->itemFactory) {
        return;
    }

    d->itemFactory->setCategoriesGroup(group);
} //<

void ActionListView::setCategoriesGroupByName(const QString & group) //>
{
    setCategoriesGroup(instance()->group(group));
} //<

WidgetGroup * ActionListView::categoriesGroup() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }

    return d->itemFactory->categoriesGroup();
} //<

void ActionListView::groupUpdated() //>
{
    Widget::groupUpdated();

    if (group()->hasProperty("ExtenderPosition")) {
        setExtenderPosition((ExtenderPosition)(group()->property("ExtenderPosition").toInt()));
    }
} //<

void ActionListView::keyPressEvent(QKeyEvent * event) //>
{
    if (!d->itemFactory) {
        return;
    }

    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        if (event->modifiers() & Qt::AltModifier) {
            d->itemFactory->contextForSelectedItem();
        } else {
            d->itemFactory->activateSelectedItem();
        }
    } else if (event->key() == Qt::Key_Down) {
        d->itemFactory->selectRelItem(+1);
    } else if (event->key() == Qt::Key_Up) {
        d->itemFactory->selectRelItem(-1);
    }
} //<

void ActionListView::clearSelection() //>
{
    d->itemFactory->clearSelection();
} //<

void ActionListView::initialSelection() //>
{
    d->itemFactory->clearSelection();
    d->itemFactory->selectRelItem(+1);
} //<

int ActionListView::selectedIndex() const //>
{
    if (!d->itemFactory->m_selectedItem) {
        return -1;
    }

    return d->itemFactory->m_items.indexOf(d->itemFactory->m_selectedItem);
} //>

bool ActionListView::areCategoriesActivable() const //>
{
    return d->itemFactory->m_categoriesActivable;
} //<

void ActionListView::setCategoriesActivable(bool value) //>
{
    d->itemFactory->m_categoriesActivable = value;
    d->itemFactory->updateExtenderPosition();
} //<

//<

} // namespace Lancelot
