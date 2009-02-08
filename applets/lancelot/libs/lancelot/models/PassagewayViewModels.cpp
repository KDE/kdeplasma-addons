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

PassagewayViewModelProxy::PassagewayViewModelProxy(ActionListModel * model, QString title, QIcon icon)
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

ActionListModel * PassagewayViewModelProxy::model() const
{
    return m_model;
}

QMimeData * PassagewayViewModel::modelMimeData()
{
    return NULL;
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

// ActionListModel
QString PassagewayViewModelProxy::title(int index) const
{
    return m_model->title(index);
}

bool PassagewayViewModelProxy::hasContextActions(int index) const
{
    return m_model->hasContextActions(index);
}

void PassagewayViewModelProxy::setContextActions(int index, Lancelot::PopupMenu * menu)
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

// StandardPassagewayViewModel
StandardPassagewayViewModel::StandardPassagewayViewModel()
    : PassagewayViewModel(), m_root(NULL)
{
    kDebug() << "##################" << (void *) this;
    if (!m_root) {
        m_root = new Item();
    }
    kDebug() << "##################" << (void *) m_root;
}

StandardPassagewayViewModel::StandardPassagewayViewModel(Item * root)
    : PassagewayViewModel(), m_root(root)
{
    if (!m_root) {
        m_root = new Item();
    }
    kDebug() << "##################" << (void *) m_root;
}

StandardPassagewayViewModel::~StandardPassagewayViewModel()
{
    qDeleteAll(childModels);
    delete m_root;
}

PassagewayViewModel * StandardPassagewayViewModel::child(int index)
{
    if (index < 0 || index >= m_root->children.size()) {
        return NULL;
    }

    Item * childItem = & m_root->children.value(index);

    if (childItem->children.size() == 0) {
        return NULL;
    }

    if (!childModels.contains(childItem)) {
        childModels[childItem] = // new StandardPassagewayViewModel(childItem);
            createChild(index);
    }

    return childModels[childItem];
}

bool StandardPassagewayViewModel::isCategory(int index) const
{
    if (index < 0 || index >= m_root->children.size()) {
        return false;
    }

    return m_root->children.at(index).children.size() != 0;
}

QString StandardPassagewayViewModel::modelTitle() const
{
    return m_root->title;
}

QIcon StandardPassagewayViewModel::modelIcon() const
{
    return m_root->icon;
}

QString StandardPassagewayViewModel::title(int index) const
{
    if (index < 0 || index >= m_root->children.size()) {
        return QString();
    }

    return m_root->children.at(index).title;
}

QString StandardPassagewayViewModel::description(int index) const
{
    if (index < 0 || index >= m_root->children.size()) {
        return QString();
    }

    return m_root->children.at(index).description;
}

QIcon StandardPassagewayViewModel::icon(int index) const
{
    if (index < 0 || index >= m_root->children.size()) {
        return QIcon();
    }

    return m_root->children.at(index).icon;
}

int StandardPassagewayViewModel::size() const
{
    kDebug() << (void *) this;
    return m_root->children.size();
}

void StandardPassagewayViewModel::add(const Item & item, Item * parent)
{
    if (parent == NULL) parent = m_root;

    parent->children << item;
}

void StandardPassagewayViewModel::add(const QString & title, const QString & description, QIcon icon, const QVariant & data, Item * parent)
{
    add(Item(title, description, icon, data), parent);
}

void StandardPassagewayViewModel::set(int index, const Item & item, Item * parent)
{
    if (parent == NULL) parent = m_root;
    if (index < 0 || index >= parent->children.size()) return;

    parent->children[index] = item;
}

void StandardPassagewayViewModel::set(int index, const QString & title, const QString & description, QIcon icon, const QVariant & data, Item * parent)
{
    set(index, Item(title, description, icon, data), parent);
}

void StandardPassagewayViewModel::removeAt(int index, Item * parent)
{
    if (parent == NULL) parent = m_root;
    parent->children.removeAt(index);
}

void StandardPassagewayViewModel::clear(Item * parent)
{
    if (parent == NULL) parent = m_root;
    parent->children.clear();
}

StandardPassagewayViewModel::Item & StandardPassagewayViewModel::itemAt(int index, Item * parent)
{
    if (parent == NULL) parent = m_root;
    return parent->children[index];
}

} // namespace Lancelot

