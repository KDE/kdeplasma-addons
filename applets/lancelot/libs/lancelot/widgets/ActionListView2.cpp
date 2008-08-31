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

    }

    L_Override virtual int itemHeight(int index, Qt::SizeHint which) const
    {

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

private:
    ActionListViewModel * m_model;
    QList < ActionListView2Item * > m_items;
};

// ActionListView2

class ActionListView2::Private {
public:

};

ActionListView2::ActionListView2(QGraphicsItem * parent)
    : CustomListView(parent), d(new Private())
{

}

ActionListView2::ActionListView2(ActionListViewModel * model, QGraphicsItem * parent)
    : CustomListView(new ActionListView2ItemFactory(model), model, parent),
      d(new Private())
{

}

ActionListView2::~ActionListView2()
{

}

void ActionListView2::setModel(ActionListViewModel * model)
{

}

ActionListViewModel * ActionListView2::model() const
{

}

void ActionListView2::setCategoriesActivable(bool value)
{

}

bool ActionListView2::categoriesActivable() const
{

}

void ActionListView2::setExtenderPosition(ExtenderPosition position)
{

}

ExtenderPosition ActionListView2::extenderPosition() const
{

}


} // namespace Lancelot
