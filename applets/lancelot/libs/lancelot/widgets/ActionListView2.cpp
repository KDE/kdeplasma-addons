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

#include "ActionListView2.h"
#include "ActionListView2_p.h"

#include <QGraphicsSceneMouseEvent>
#include <QApplication>

namespace Lancelot {

//> ActionListView2Item
ActionListView2Item::ActionListView2Item(ActionListView2ItemFactory * factory)
    : ExtenderButton(), m_inSetSelected(false), m_factory(factory)
{
    connect(this, SIGNAL(mouseHoverEnter()),
            this, SLOT(select()));
    connect(this, SIGNAL(mouseHoverLeave()),
            this, SLOT(deselect()));
    L_WIDGET_SET_INITIALIZED;
}

ActionListView2Item::~ActionListView2Item()
{
}

void ActionListView2Item::select()
{
    setSelected(true);
}

void ActionListView2Item::deselect()
{
    setSelected(false);
    m_factory->m_selectedItem = NULL;
}

void ActionListView2Item::setSelected(bool selected)
{
    if (m_inSetSelected) return;
    m_inSetSelected = true;

    setHovered(selected);

    m_factory->setSelectedItem(this, selected);

    if (!selected) {
        hoverLeaveEvent(NULL);
    } else {
        m_factory->m_view->scrollTo(geometry());
        hoverEnterEvent(NULL);
    }

    m_inSetSelected = false;
}

bool ActionListView2Item::isSelected() const
{
    return m_factory->m_selectedItem == this;
}

void ActionListView2Item::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
    Q_UNUSED(event);
    m_factory->itemContext(this);
}

void ActionListView2Item::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_mousePos = event->pos();
    ExtenderButton::mousePressEvent(event);
}

void ActionListView2Item::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    ExtenderButton::mouseMoveEvent(event);
    if (isDown() && ((m_mousePos - event->pos()).toPoint().manhattanLength() > QApplication::startDragDistance())) {
        setDown(false);
        m_factory->itemDrag(this, event->widget());
    }
}

//<

//> ActionListView2ItemFactory
ActionListView2ItemFactory::ActionListView2ItemFactory(ActionListViewModel * model, ActionListView2 * view, Instance * instance) //>
    : m_model(NULL),
      m_extenderPosition(NoExtender),
      m_itemsGroup(NULL), m_categoriesGroup(NULL),
      m_instance(instance), m_view(view),
      m_categoriesActivable(false), m_selectedItem(NULL)
{
    setItemsGroup(NULL);
    setCategoriesGroup(NULL);
    setModel(model);
} //<

ActionListView2ItemFactory::~ActionListView2ItemFactory() //>
{
    qDeleteAll(m_items);
    m_items.clear();
} //<

void ActionListView2ItemFactory::reload() //>
{
    while (m_items.size() > m_model->size()) {
        kDebug() << "deleting one";
        ActionListView2Item * item = m_items.takeLast();
        item->hide();
        item->deleteLater();
    }

    kDebug() << "reloading the items";
    for (int i = 0; i < m_model->size(); i++) {
        itemForIndex(i, true);
    }

    emit updated();
} //<

CustomListItem * ActionListView2ItemFactory::itemForIndex(int index) //>
{
    return itemForIndex(index, false);
} //<

CustomListItem * ActionListView2ItemFactory::itemForIndex(int index,
        bool reload) //>
{
    ActionListView2Item * item;
    kDebug() << "req index:" << index
             << "items size:" << m_items.size()
             << "model size:" << m_model->size();
    if (index < m_items.size() && m_items[index]) {
        item = m_items[index];
    } else {
        kDebug() << "Creating new one";
        item = new ActionListView2Item(this);
        item->setGroup((m_model->isCategory(index))
                ? m_categoriesGroup : m_itemsGroup);
        reload = true;
        while (index >= m_items.size()) {
            m_items.append(NULL);
            kDebug() << "Extending items list to fit item";
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
            item->setIconSize(QSize(20, 20));
        } else {
            item->setGroup(m_itemsGroup);
        }
    }

    return item;
} //<

void ActionListView2ItemFactory::setItemsGroup(WidgetGroup * group) //>
{
    if (group == NULL) {
        group = m_instance->group("ActionListView-Items");
    }

    if (group == m_itemsGroup) return;

    m_itemsGroup = group;

    int i = 0;
    foreach(ActionListView2Item * item, m_items) {
        if (!(m_model->isCategory(i))) {
            item->setGroup(group);
        }
        i++;
    }
}

WidgetGroup * ActionListView2ItemFactory::itemsGroup() const //>
{
    return m_itemsGroup;
} //<

void ActionListView2ItemFactory::setCategoriesGroup(WidgetGroup * group) //>
{
    if (group == NULL) {
        group = m_instance->group("ActionListView-Categories");
    }

    if (group == m_itemsGroup) return;

    m_categoriesGroup = group;

    int i = 0;
    foreach(ActionListView2Item * item, m_items) {
        if (m_model->isCategory(i)) {
            item->setGroup(group);
            setItemExtender(i);
        }
        i++;
    }
}

WidgetGroup * ActionListView2ItemFactory::categoriesGroup() const //>
{
    return m_categoriesGroup;
} //<

void ActionListView2ItemFactory::itemActivated() //>
{
    if (!sender()) {
        return;
    }

    activate(m_items.indexOf(
                (Lancelot::ActionListView2Item *)sender()));
} //<

void ActionListView2ItemFactory::activate(int index) //>
{
    if (index < 0 || index >= m_model->size()) {
        return;
    }
    m_model->activated(index);
    emit activated(index);
} //<

int ActionListView2ItemFactory::itemCount() const //>
{
    if (m_model) {
        return m_model->size();
    } else {
        return 0;
    }
} //<

int ActionListView2ItemFactory::itemHeight(int index, Qt::SizeHint which) const //>
{
    if (m_model->isCategory(index)) {
        switch (which) {
            case Qt::MinimumSize:
                return 20;
            case Qt::MaximumSize:
                return 35;
            default:
                return 27;
        }
    } else {
        switch (which) {
            case Qt::MinimumSize:
                return 40;
            case Qt::MaximumSize:
                return 70;
            default:
                return 55;
        }
    }
} //<

void ActionListView2ItemFactory::setModel(ActionListViewModel * model) //>
{
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

} //<

ActionListViewModel * ActionListView2ItemFactory::model() //>
{
    return m_model;
} //<

void ActionListView2ItemFactory::modelUpdated() //>
{
    kDebug();
    reload();
} //<

void ActionListView2ItemFactory::modelItemInserted(int index) //>
{
    if (index < 0 || index > m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        kDebug() << "illegal -> reloading: " << index;
        reload();
    } else {
        m_items.insert(index, NULL);
        itemForIndex(index, true);
        kDebug() << "emit itemInserted" << index;
        emit itemInserted(index);
    }
} //<

void ActionListView2ItemFactory::modelItemDeleted(int index) //>
{
    kDebug();
    if (index < 0 || index >= m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        delete m_items.takeAt(index);
        emit itemDeleted(index);
    }
} //<

void ActionListView2ItemFactory::modelItemAltered(int index) //>
{
    kDebug();
    kDebug();
    if (index < 0 || index >= m_items.size()) {
        // If we get an illegal notification, do
        // a full reload
        reload();
    } else {
        itemForIndex(index, true);
        emit itemAltered(index);
    }
} //<

void ActionListView2ItemFactory::setExtenderPosition(ExtenderPosition position) //>
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

void ActionListView2ItemFactory::updateExtenderPosition() //>
{
    for (int i = 0; i < m_items.count(); i++) {
        setItemExtender(i);
    }
} //<

void ActionListView2ItemFactory::setItemExtender(int index) //>
{
    ActionListView2Item * item = m_items.at(index);
    if (m_model->isCategory(index) && !m_categoriesActivable) {
        item->setExtenderPosition(NoExtender);
    } else {
        item->setExtenderPosition(m_extenderPosition);
    }
} //<

ExtenderPosition ActionListView2ItemFactory::extenderPosition() const //>
{
    return m_extenderPosition;
} //<

void ActionListView2ItemFactory::itemContext(ActionListView2Item * sender) //>
{
    int index = m_items.indexOf(sender);
    if (index < 0 || index >= m_model->size() ||
            !m_model->hasContextActions(index)) {
        return;
    }

    QMenu menu;
    m_model->setContextActions(index, &menu);
    m_model->contextActivate(index, menu.exec(QCursor::pos()));

} //<

void ActionListView2ItemFactory::itemDrag(ActionListView2Item * sender, QWidget * widget) //>
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

void ActionListView2ItemFactory::activateSelectedItem() //>
{
    kDebug() << (void *) m_selectedItem;
    if (!m_selectedItem) {
        return;
    }

    kDebug() << m_items.indexOf(m_selectedItem);
    activate(m_items.indexOf(m_selectedItem));
} //<

void ActionListView2ItemFactory::clearSelection() //>
{
    if (m_selectedItem) {
        setSelectedItem(m_selectedItem, false);
    }
}

void ActionListView2ItemFactory::setSelectedItem(ActionListView2Item * item, bool selected) //>
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

void ActionListView2ItemFactory::selectRelItem(int rel) //>
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

//> ActionListView2
ActionListView2::Private::Private() //>
    : itemFactory(NULL)
{
} //<

ActionListView2::Private::~Private() //>
{
    delete itemFactory;
} //<

ActionListView2::ActionListView2(QGraphicsItem * parent) //>
    : CustomListView(parent), d(new Private())
{
    setFlag(ScrollPane::HoverShowScrollbars);
    clearFlag(ScrollPane::ClipScrollable);
    setFocusPolicy(Qt::WheelFocus);

    L_WIDGET_SET_INITIALIZED;
} //<

ActionListView2::ActionListView2(ActionListViewModel * model, QGraphicsItem * parent) //>
    : CustomListView(parent),
      d(new Private())
{
    setModel(model);
    // d->itemFactory = (ActionListView2ItemFactory *) list()->itemFactory();
    connect(
            d->itemFactory, SIGNAL(activated(int)),
            this, SIGNAL(activated(int)));

    setFlag(ScrollPane::HoverShowScrollbars);
    clearFlag(ScrollPane::ClipScrollable);
    setFocusPolicy(Qt::WheelFocus);

    L_WIDGET_SET_INITIALIZED;
} //<

ActionListView2::~ActionListView2() //>
{
    delete d;
} //<

void ActionListView2::setModel(ActionListViewModel * model) //>
{
    if (!d->itemFactory) {
        d->itemFactory = new ActionListView2ItemFactory(model, this, instance());
        list()->setItemFactory(d->itemFactory);
    } else {
        d->itemFactory->setModel(model);
    }
} //<

ActionListViewModel * ActionListView2::model() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }
    return d->itemFactory->model();
} //<

void ActionListView2::setExtenderPosition(ExtenderPosition position) //>
{
    if (!d->itemFactory) {
        return;
    }
    d->itemFactory->setExtenderPosition(position);

    if (d->itemFactory->extenderPosition() == LeftExtender) {
        list()->setMargin(Plasma::LeftMargin, EXTENDER_SIZE);
        list()->setMargin(Plasma::RightMargin, 0);
        setFlip(Plasma::NoFlip);
    } else if (d->itemFactory->extenderPosition() == RightExtender) {
        list()->setMargin(Plasma::LeftMargin, 0);
        list()->setMargin(Plasma::RightMargin, EXTENDER_SIZE);
        setFlip(Plasma::HorizontalFlip);
    } else {
        list()->setMargin(Plasma::LeftMargin, 0);
        list()->setMargin(Plasma::RightMargin, 0);
        setFlip(Plasma::NoFlip);
    }

} //<

ExtenderPosition ActionListView2::extenderPosition() const //>
{
    if (!d->itemFactory) {
        return NoExtender;
    }
    return d->itemFactory->extenderPosition();
} //<

void ActionListView2::setItemsGroup(WidgetGroup * group) //>
{
    if (!d->itemFactory) {
        return;
    }

    d->itemFactory->setItemsGroup(group);
} //<

void ActionListView2::setItemsGroupByName(const QString & group) //>
{
    setItemsGroup(instance()->group(group));
} //<

WidgetGroup * ActionListView2::itemsGroup() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }

    return d->itemFactory->itemsGroup();
} //<

void ActionListView2::setCategoriesGroup(WidgetGroup * group) //>
{
    if (!d->itemFactory) {
        return;
    }

    d->itemFactory->setCategoriesGroup(group);
} //<

void ActionListView2::setCategoriesGroupByName(const QString & group) //>
{
    setCategoriesGroup(instance()->group(group));
} //<

WidgetGroup * ActionListView2::categoriesGroup() const //>
{
    if (!d->itemFactory) {
        return NULL;
    }

    return d->itemFactory->categoriesGroup();
} //<

void ActionListView2::groupUpdated() //>
{
    Widget::groupUpdated();

    if (group()->hasProperty("ExtenderPosition")) {
        setExtenderPosition((ExtenderPosition)(group()->property("ExtenderPosition").toInt()));
    }
} //<

void ActionListView2::keyPressEvent(QKeyEvent * event) //>
{
    if (!d->itemFactory) {
        return;
    }

    kDebug() << event->key() << Qt::Key_Enter;
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        d->itemFactory->activateSelectedItem();
    } else if (event->key() == Qt::Key_Down) {
        d->itemFactory->selectRelItem(+1);
    } else if (event->key() == Qt::Key_Up) {
        d->itemFactory->selectRelItem(-1);
    }
} //<

void ActionListView2::clearSelection() //>
{
    d->itemFactory->clearSelection();
} //<

void ActionListView2::initialSelection() //>
{
    d->itemFactory->clearSelection();
    d->itemFactory->selectRelItem(+1);
} //<

int ActionListView2::selectedIndex() const //>
{
    if (!d->itemFactory->m_selectedItem) {
        return -1;
    }

    return d->itemFactory->m_items.indexOf(d->itemFactory->m_selectedItem);
} //>

bool ActionListView2::areCategoriesActivable() const //>
{
    return d->itemFactory->m_categoriesActivable;
} //<

void ActionListView2::setCategoriesActivable(bool value) //>
{
    d->itemFactory->m_categoriesActivable = value;
    d->itemFactory->updateExtenderPosition();
} //<

//<

} // namespace Lancelot
