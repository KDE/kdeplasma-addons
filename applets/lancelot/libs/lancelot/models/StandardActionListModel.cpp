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

#include "StandardActionListModel.h"

namespace Lancelot {

StandardActionListModel::StandardActionListModel()
    : m_sendEmits(true)
{
}

StandardActionListModel::~StandardActionListModel()
{
}

void StandardActionListModel::setEmitInhibited(bool value)
{
    m_sendEmits = !value;
}

bool StandardActionListModel::emitInhibited() const
{
    return !m_sendEmits;
}

QString StandardActionListModel::title(int index) const
{
    if (index >= m_items.size()) return "";
    return m_items.at(index).title;
}

QString StandardActionListModel::description(int index) const
{
    if (index >= m_items.size()) return "";
    return m_items.at(index).description;
}

QIcon StandardActionListModel::icon(int index) const
{
    if (index >= m_items.size()) return QIcon();
    return m_items.at(index).icon;
}

bool StandardActionListModel::isCategory(int index) const
{
    Q_UNUSED(index);
    return false;
}

int StandardActionListModel::size() const
{
    return m_items.size();
}

void StandardActionListModel::add(const Item & item)
{
    m_items.append(item);
    if (m_sendEmits) {
        emit itemInserted(m_items.size() - 1);
    }
}

void StandardActionListModel::add(const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    add(Item(title, description, icon, data));
}

void StandardActionListModel::set(int index, const Item & item)
{
    if (index < 0 || index >= m_items.size()) return;
    m_items[index] = item;
    if (m_sendEmits) {
        emit itemAltered(index);
    }
}

void StandardActionListModel::set(int index, const QString & title, const QString & description, QIcon icon, const QVariant & data)
{
    set(index, Item(title, description, icon, data));
}

void StandardActionListModel::removeAt(int index)
{
    m_items.removeAt(index);
    if (m_sendEmits) {
        emit itemDeleted(index);
    }
}

void StandardActionListModel::clear()
{
    m_items.clear();
    if (m_sendEmits) {
        emit updated();
    }
}

StandardActionListModel::Item & StandardActionListModel::itemAt(int index)
{
    return m_items[index];
}

} // namespace Lancelot

#include "StandardActionListModel.moc"

