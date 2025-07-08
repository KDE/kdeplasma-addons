/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "abstractunitlistmodel.h"

AbstractUnitListModel::AbstractUnitListModel(const QList<UnitItem> &items, int defaultUnitId, QObject *parent)
    : QAbstractListModel(parent)
    , m_items(items)
    , m_defaultIndex(listIndexForUnitId(defaultUnitId))
{
}

QVariant AbstractUnitListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }

    const UnitItem &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return item.name;
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

    return m_defaultIndex;
}

int AbstractUnitListModel::unitIdForListIndex(int listIndex) const
{
    if (0 <= listIndex && listIndex < m_items.count()) {
        return m_items.at(listIndex).unitId;
    }

    return -1;
}

int AbstractUnitListModel::defaultUnitId() const
{
    return unitIdForListIndex(m_defaultIndex);
}

#include "moc_abstractunitlistmodel.cpp"
