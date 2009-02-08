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

#include "ActionTreeModelProxy.h"

namespace Lancelot
{

ActionTreeModelProxy::ActionTreeModelProxy(ActionListModel * model, QString title, QIcon icon)
    : m_model(model), m_modelTitle(title), m_modelIcon(icon)
{
    connect(model, SIGNAL( itemActivated(int) ),
            this,  SIGNAL( itemActivated(int) ));
    connect(model, SIGNAL( itemInserted(int)  ),
            this,  SIGNAL( itemInserted(int)  ));
    connect(model, SIGNAL( itemAltered(int)   ),
            this,  SIGNAL( itemAltered(int)   ));
    connect(model, SIGNAL( itemDeleted(int)   ),
            this,  SIGNAL( itemDeleted(int)   ));
    connect(model, SIGNAL( updated()          ),
            this,  SIGNAL( updated()          ));
}

ActionListModel * ActionTreeModelProxy::model() const
{
    return m_model;
}

QMimeData * ActionTreeModel::modelMimeData()
{
    return NULL;
}

// ActionTreeModel
ActionTreeModel * ActionTreeModelProxy::child(int index)
{
    return NULL;
}

QString ActionTreeModelProxy::modelTitle() const
{
    return m_modelTitle;
}

QIcon ActionTreeModelProxy::modelIcon()  const
{
    return m_modelIcon;
}

// ActionListModel
QString ActionTreeModelProxy::title(int index) const
{
    return m_model->title(index);
}

bool ActionTreeModelProxy::hasContextActions(int index) const
{
    return m_model->hasContextActions(index);
}

void ActionTreeModelProxy::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    m_model->setContextActions(index, menu);
}

void ActionTreeModelProxy::contextActivate(int index, QAction * context)
{
    m_model->contextActivate(index, context);
}

QString ActionTreeModelProxy::description(int index) const
{
    return m_model->description(index);
}

QIcon ActionTreeModelProxy::icon(int index) const
{
    return m_model->icon(index);
}

bool ActionTreeModelProxy::isCategory(int index) const
{
    return m_model->isCategory(index);
}

int ActionTreeModelProxy::size() const
{
    return m_model->size();
}

void ActionTreeModelProxy::activate(int index)
{
    m_model->activated(index);
}

QMimeData * ActionTreeModelProxy::mimeData(int index) const
{
    return m_model->mimeData(index);
}

void ActionTreeModelProxy::setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    m_model->setDropActions(index, actions, defaultAction);
}

} // namespace Lancelot

