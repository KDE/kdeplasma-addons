/*
 * Copyright (C) 2017 David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "dictionariesmodel.h"
#include <Plasma/DataEngine>
#include <Plasma/DataContainer>
#include <QDebug>

DictionariesModel::DictionariesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    Plasma::DataEngine* dataengine = dataEngine(QStringLiteral("dict"));
    const QString source = QLatin1String("list-dictionaries");
    Plasma::DataContainer *container = dataengine->containerForSource(source);
    if (container) { // in practice this never seems to happen, this source is only used here, so never shared
        setAvailableDicts(container->data());
    }
    dataengine->connectSource(source, this);
}

QVariant DictionariesModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();
    switch (role) {
    case Qt::DisplayRole:
        return m_availableDicts[row].description;
    case Qt::EditRole:
        return m_availableDicts[row].id;
    default:
        break;
    }
    return QVariant();
}

int DictionariesModel::rowCount(const QModelIndex& index) const
{
    if (index.isValid())
        return 0; // flat model
    return m_availableDicts.size();
}

QHash<int, QByteArray> DictionariesModel::roleNames() const
{
    return { { Qt::DisplayRole, "description" }, { Qt::EditRole, "id" } };
}

void DictionariesModel::dataUpdated(const QString& sourceName, const Plasma::DataEngine::Data& data)
{
    Q_ASSERT(sourceName == QLatin1String("list-dictionaries"));
    beginResetModel();
    setAvailableDicts(data);
    endResetModel();
}

void DictionariesModel::setAvailableDicts(const QVariantMap& data)
{
    m_availableDicts = {};
    m_availableDicts.resize(data.count());
    int i = 0;
    for (auto it = data.begin(), end = data.end(); it != end; ++it, ++i) {
        m_availableDicts[i] = AvailableDict{it.key(), it.value().toString()};
    }
}
