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

#include "StandardActionListModel.h"

namespace Lancelot {

class StandardActionListModel::Private {
public:
    bool sendEmits;
    QList < Item > items;
};

StandardActionListModel::StandardActionListModel()
    : d(new Private())
{
    d->sendEmits = true;
}

StandardActionListModel::~StandardActionListModel()
{
    delete d;
}

void StandardActionListModel::setEmitInhibited(bool value)
{
    d->sendEmits = !value;
}

bool StandardActionListModel::emitInhibited() const
{
    return !d->sendEmits;
}

QString StandardActionListModel::title(int index) const
{
    if (index >= d->items.size()) return "";

    QString result = d->items.at(index).title;

    result.replace('&', "&&");
    return result;
}

QString StandardActionListModel::description(int index) const
{
    if (index >= d->items.size()) return "";
    return d->items.at(index).description;
}

QIcon StandardActionListModel::icon(int index) const
{
    if (index >= d->items.size()) return QIcon();
    return d->items.at(index).icon;
}

bool StandardActionListModel::isCategory(int index) const
{
    Q_UNUSED(index);
    return false;
}

int StandardActionListModel::size() const
{
    return d->items.size();
}

void StandardActionListModel::add(const Item & item)
{
    d->items.append(item);
    if (d->sendEmits) {
        emit itemInserted(d->items.size() - 1);
    }
}

void StandardActionListModel::add(const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    add(Item(title, description, icon, data));
}

void StandardActionListModel::insert(int where, const Item & item)
{
    d->items.insert(where, item);
    if (d->sendEmits) {
        emit itemInserted(where);
    }
}

void StandardActionListModel::insert(int where, const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    insert(where, Item(title, description, icon, data));
}

void StandardActionListModel::set(int index, const Item & item)
{
    if (index < 0 || index >= d->items.size()) return;
    d->items[index] = item;
    if (d->sendEmits) {
        emit itemAltered(index);
    }
}

void StandardActionListModel::set(int index, const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    set(index, Item(title, description, icon, data));
}

void StandardActionListModel::removeAt(int index)
{
    d->items.removeAt(index);
    if (d->sendEmits) {
        emit itemDeleted(index);
    }
}

void StandardActionListModel::clear()
{
    d->items.clear();
    if (d->sendEmits) {
        emit updated();
    }
}

const StandardActionListModel::Item & StandardActionListModel::itemAt(int index) const
{
    return d->items[index];
}

} // namespace Lancelot

#include "StandardActionListModel.moc"

