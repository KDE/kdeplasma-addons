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

namespace Lancelot {

//> ActionListView2Item
ActionListView2Item::ActionListView2Item()
    : ExtenderButton(), m_selected(false)
{
    L_WIDGET_SET_INITIALIZED;
}

ActionListView2Item::~ActionListView2Item()
{
}

void ActionListView2Item::setSelected(bool selected)
{
    m_selected = selected;
}

bool ActionListView2Item::isSelected() const
{
    return m_selected;
}
//<

//> ActionListView2ItemFactory
ActionListView2ItemFactory::ActionListView2ItemFactory(ActionListViewModel * model)
    : m_model(NULL), m_categoriesActivable(false)
{
    setModel(model);
}

ActionListView2ItemFactory::~ActionListView2ItemFactory()
{
    foreach (ActionListView2Item * item, m_items) {
        delete item;
    }
    m_items.clear();
}

void ActionListView2ItemFactory::reload()
{
    kDebug() << "reloading the items";
    for (int i = 0; i < m_model->size(); i++) {
        itemForIndex(i, true);
    }

    kDebug() << "model size:" << m_model->size()
             << "items size:" << m_items.size();
    for (int i = m_model->size(); i <= m_items.size(); i++) {
        kDebug() << "deleting one";
        delete m_items.takeLast();
    }
    kDebug() << "model size:" << m_model->size()
             << "items size:" << m_items.size();
}

CustomListItem * ActionListView2ItemFactory::itemForIndex(int index)
{
    return itemForIndex(index, false);
}

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
        item = new ActionListView2Item();
        reload = true;
        while (index >= m_items.size()) {
            m_items.append(NULL);
            kDebug() << "Extending items list to fit item";
        }
        m_items[index] = item;
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
            if (m_categoriesActivable) {
                item->setGroupByName("ActionListView-CategoriesPass");
            } else {
                item->setGroupByName("ActionListView-Categories");
            }
            item->setIconSize(QSize(20, 20));
        } else {
            item->setGroupByName("ActionListView-Items");
        }
    }

    return item;
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

bool ActionListView2ItemFactory::categoriesActivable() const //>
{
    return m_categoriesActivable;
} //<

void ActionListView2ItemFactory::setCategoriesActivable(bool value) //>
{
    if (value == m_categoriesActivable) {
        return;
    }

    m_categoriesActivable = value;

    for (int i = 0; i < m_items.size(); i++) {
        if (m_model->isCategory(i)) {
            if (m_categoriesActivable) {
                m_items[i]->setGroupByName("ActionListView-CategoriesPass");
            } else {
                m_items[i]->setGroupByName("ActionListView-Categories");
            }
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

void ActionListView2ItemFactory::modelUpdated()
{
    reload();
}

void ActionListView2ItemFactory::modelItemInserted(int index)
{
    kDebug();
    m_items.insert(index, NULL);
    itemForIndex(index, true);
    emit itemInserted(index);
}

void ActionListView2ItemFactory::modelItemDeleted(int index)
{
    kDebug();
    delete m_items.takeAt(index);
    emit itemDeleted(index);
}

void ActionListView2ItemFactory::modelItemAltered(int index)
{
    kDebug();
    itemForIndex(index, true);
    emit itemAltered(index);
}
//<

//> ActionListView2
ActionListView2::Private::Private()
    : itemFactory(NULL)
{
}

ActionListView2::Private::~Private()
{
    delete itemFactory;
}

ActionListView2::ActionListView2(QGraphicsItem * parent)
    : CustomListView(parent), d(new Private())
{
    setFlag(ScrollPane::HoverShowScrollbars);
    clearFlag(ScrollPane::ClipScrollable);

    L_WIDGET_SET_INITIALIZED;
}

ActionListView2::ActionListView2(ActionListViewModel * model, QGraphicsItem * parent)
    : CustomListView(new ActionListView2ItemFactory(model), parent),
      d(new Private())
{
    d->itemFactory = (ActionListView2ItemFactory *) list()->itemFactory();
    setFlag(ScrollPane::HoverShowScrollbars);
    clearFlag(ScrollPane::ClipScrollable);

    L_WIDGET_SET_INITIALIZED;
}

ActionListView2::~ActionListView2()
{
    delete d;
}

void ActionListView2::setCategoriesActivable(bool value)
{
    if (d->itemFactory) {
        d->itemFactory->setCategoriesActivable(value);
    }
}

bool ActionListView2::categoriesActivable() const
{
    if (d->itemFactory) {
        return d->itemFactory->categoriesActivable();
    }
    return false;
}

void ActionListView2::setModel(ActionListViewModel * model)
{
    if (!d->itemFactory) {
        d->itemFactory = new ActionListView2ItemFactory(model);
        list()->setItemFactory(d->itemFactory);
    } else {
        d->itemFactory->setModel(model);
    }
}

ActionListViewModel * ActionListView2::model() const
{
    if (!d->itemFactory) {
        return NULL;
    }
    return d->itemFactory->model();
}
//<

} // namespace Lancelot
