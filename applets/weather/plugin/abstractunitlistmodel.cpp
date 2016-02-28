/*
 * Copyright 2016  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "abstractunitlistmodel.h"

AbstractUnitListModel::AbstractUnitListModel(const QVector<UnitItem> &items, QObject *parent)
    : QAbstractListModel(parent)
    , m_items(items)
{
}

QVariant AbstractUnitListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }

    const UnitItem &item = m_items.at(index.row());

    switch (role) {
        case Qt::DisplayRole: return item.name;
    }

    return QVariant();
}

int AbstractUnitListModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_items.size();
    }

    return 0;
}

int AbstractUnitListModel::listIndexForUnitId(int unitId) const
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).unitId == unitId) {
            return i;
        }
    }

    return -1;
}

int AbstractUnitListModel::unitIdForListIndex(int listIndex) const
{
    if (0 <= listIndex && listIndex < m_items.count()) {
        return m_items.at(listIndex).unitId;
    }

    return -1;
}
