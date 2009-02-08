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

#include "StandardActionTreeModel.h"

namespace Lancelot
{

class StandardActionTreeModel::Private {
public:
    Item * root;
    bool deleteRoot;
    QHash < Item * , StandardActionTreeModel * > childModels;
};

StandardActionTreeModel::StandardActionTreeModel()
    : ActionTreeModel(), d(new Private())
{
    d->root = new Item();
    d->deleteRoot = true;
}

StandardActionTreeModel::StandardActionTreeModel(Item * root)
    : ActionTreeModel(), d(new Private())
{
    if (!root) {
        d->root = new Item();
        d->deleteRoot = true;
    } else {
        d->root = root;
    }
}

StandardActionTreeModel::~StandardActionTreeModel()
{
    qDeleteAll(d->childModels);
    if (d->deleteRoot) {
        delete d->root;
    }
    delete d;
}

StandardActionTreeModel::Item * StandardActionTreeModel::root() const
{
    return d->root;
}

ActionTreeModel * StandardActionTreeModel::child(int index)
{
    if (index < 0 || index >= d->root->children.size()) {
        return NULL;
    }

    Item * childItem = & d->root->children.value(index);

    if (childItem->children.size() == 0) {
        return NULL;
    }

    if (!d->childModels.contains(childItem)) {
        d->childModels[childItem] = // new StandardActionTreeModel(childItem);
            createChild(index);
    }

    return d->childModels[childItem];
}

bool StandardActionTreeModel::isCategory(int index) const
{
    if (index < 0 || index >= d->root->children.size()) {
        return false;
    }

    return d->root->children.at(index).children.size() != 0;
}

QString StandardActionTreeModel::modelTitle() const
{
    return d->root->title;
}

QIcon StandardActionTreeModel::modelIcon() const
{
    return d->root->icon;
}

QString StandardActionTreeModel::title(int index) const
{
    if (index < 0 || index >= d->root->children.size()) {
        return QString();
    }

    return d->root->children.at(index).title;
}

QString StandardActionTreeModel::description(int index) const
{
    if (index < 0 || index >= d->root->children.size()) {
        return QString();
    }

    return d->root->children.at(index).description;
}

QIcon StandardActionTreeModel::icon(int index) const
{
    if (index < 0 || index >= d->root->children.size()) {
        return QIcon();
    }

    return d->root->children.at(index).icon;
}

int StandardActionTreeModel::size() const
{
    kDebug() << (void *) this;
    return d->root->children.size();
}

void StandardActionTreeModel::add(const Item & item, Item * parent)
{
    if (parent == NULL) parent = d->root;

    parent->children << item;
}

void StandardActionTreeModel::add(const QString & title, const QString & description, QIcon icon, const QVariant & data, Item * parent)
{
    add(Item(title, description, icon, data), parent);
}

void StandardActionTreeModel::set(int index, const Item & item, Item * parent)
{
    if (parent == NULL) parent = d->root;
    if (index < 0 || index >= parent->children.size()) return;

    parent->children[index] = item;
}

void StandardActionTreeModel::set(int index, const QString & title, const QString & description, QIcon icon, const QVariant & data, Item * parent)
{
    set(index, Item(title, description, icon, data), parent);
}

void StandardActionTreeModel::removeAt(int index, Item * parent)
{
    if (parent == NULL) parent = d->root;
    parent->children.removeAt(index);
}

void StandardActionTreeModel::clear(Item * parent)
{
    if (parent == NULL) parent = d->root;
    parent->children.clear();
}

StandardActionTreeModel::Item & StandardActionTreeModel::itemAt(int index, Item * parent)
{
    if (parent == NULL) parent = d->root;
    return parent->children[index];
}

} // namespace Lancelot

