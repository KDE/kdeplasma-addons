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

#include "PassagewayViewModels.h"

namespace Lancelot
{

PassagewayViewModel::PassagewayViewModel()
{
}

PassagewayViewModel::~PassagewayViewModel()
{
}

PassagewayViewModelProxy::PassagewayViewModelProxy(ActionListViewModel * model, QString title, QIcon icon)
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

// PassagewayViewModel
PassagewayViewModel * PassagewayViewModelProxy::child(int index)
{
    return NULL;
}

QString PassagewayViewModelProxy::modelTitle() const
{
    return m_modelTitle;
}

QIcon PassagewayViewModelProxy::modelIcon()  const
{
    return m_modelIcon;
}

// ActionListViewModel
QString PassagewayViewModelProxy::title(int index) const
{
    return m_model->title(index);
}

bool PassagewayViewModelProxy::hasContextActions(int index) const
{
    return m_model->hasContextActions(index);
}

void PassagewayViewModelProxy::setContextActions(int index, QMenu * menu)
{
    m_model->setContextActions(index, menu);
}

void PassagewayViewModelProxy::contextActivate(int index, QAction * context)
{
    m_model->contextActivate(index, context);
}

QString PassagewayViewModelProxy::description(int index) const
{
    return m_model->description(index);
}

QIcon PassagewayViewModelProxy::icon(int index) const
{
    return m_model->icon(index);
}

bool PassagewayViewModelProxy::isCategory(int index) const
{
    return m_model->isCategory(index);
}

int PassagewayViewModelProxy::size() const
{
    return m_model->size();
}

void PassagewayViewModelProxy::activate(int index)
{
    m_model->activated(index);
}

QMimeData * PassagewayViewModelProxy::mimeData(int index) const
{
    return m_model->mimeData(index);
}

void PassagewayViewModelProxy::setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    m_model->setDropActions(index, actions, defaultAction);
}

} // namespace Lancelot

