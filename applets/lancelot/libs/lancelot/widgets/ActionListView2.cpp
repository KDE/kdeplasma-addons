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

namespace Lancelot {

class ActionListView2Item: public ExtenderButton, public CustomListItem {
public:
    ActionListView2Item()
        : ExtenderButton(), m_selected(false)
    {
        L_WIDGET_SET_INITIALIZED;
    }

    ~ActionListView2Item()
    {
    }

    L_Override virtual void setSelected(bool selected = true)
    {
        m_selected = selected;
    }

    L_Override virtual bool isSelected() const
    {
        return m_selected;
    }

    L_Override virtual void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0)
    {
        painter->fillRect(
                QRectF(QPointF(), size()),
                QBrush(QColor(250, 200, 100))
                );
        painter->fillRect(
                QRectF(QPointF(5, 5), size() - QSizeF(10, 10)),
                QBrush(QColor(100, 200, 100))
                );
        ExtenderButton::paint(painter, option, widget);
    }

private:
    bool m_selected;
};

class ActionListView2ItemFactory: public CustomListItemFactory {
public:
    ActionListView2ItemFactory(ActionListViewModel * model)
        : m_model(model)
    {
    }

    ~ActionListView2ItemFactory()
    {
        freeAllItems();
    }

    L_Override virtual CustomListItem * itemForIndex(int index)
    {
        ActionListView2Item * item = new ActionListView2Item();
        item->setTitle(m_model->title(index));
        item->setDescription(m_model->description(index));
        item->setIcon(m_model->icon(index));
        item->setMinimumHeight(itemHeight(index, Qt::MinimumSize));
        item->setPreferredHeight(itemHeight(index, Qt::PreferredSize));
        item->setMaximumHeight(itemHeight(index, Qt::MaximumSize));
        kDebug() << index << m_model->title(index);
        return item;
    }

    L_Override virtual int itemHeight(int index, Qt::SizeHint which) const
    {
        if (m_model->isCategory(index)) {
            switch (which) {
                case Qt::MinimumSize:
                    return 20;
                case Qt::MaximumSize:
                    return 40;
                default:
                    return 30;
            }
        } else {
            switch (which) {
                case Qt::MinimumSize:
                    return 30;
                case Qt::MaximumSize:
                    return 70;
                default:
                    return 50;
            }
        }
    }

    L_Override virtual void freeItem(int index)
    {
        if (m_items[index] != NULL) {
            delete m_items[index];
            m_items[index] = NULL;
        }
    }

    L_Override virtual void freeAllItems()
    {
        foreach (ActionListView2Item * item, m_items) {
            delete item;
        }
    }

    void setModel(ActionListViewModel * model)
    {
        m_model = model;
    }

    ActionListViewModel * model()
    {
        return m_model;
    }

private:
    ActionListViewModel * m_model;
    QList < ActionListView2Item * > m_items;
};

// ActionListView2

class ActionListView2::Private {
public:
    Private()
        : itemFactory(NULL)
    {
    }

    ~Private()
    {
        delete itemFactory;
    }

    ActionListView2ItemFactory * itemFactory;
};

ActionListView2::ActionListView2(QGraphicsItem * parent)
    : CustomListView(parent), d(new Private())
{
    L_WIDGET_SET_INITIALIZED;
}

ActionListView2::ActionListView2(ActionListViewModel * model, QGraphicsItem * parent)
    : CustomListView(new ActionListView2ItemFactory(model), model, parent),
      d(new Private())
{
    d->itemFactory = (ActionListView2ItemFactory *) list()->itemFactory();
    L_WIDGET_SET_INITIALIZED;
}

ActionListView2::~ActionListView2()
{
    delete d;
}

void ActionListView2::setModel(ActionListViewModel * model)
{
    if (!d->itemFactory) {
        d->itemFactory = new ActionListView2ItemFactory(model);
        list()->setItemFactory(d->itemFactory);
        list()->setModel(model);
    } else {
        d->itemFactory->setModel(model);
        list()->setModel(model);
    }
}

ActionListViewModel * ActionListView2::model() const
{
    if (!d->itemFactory) {
        return NULL;
    }
    return d->itemFactory->model();
}

} // namespace Lancelot
