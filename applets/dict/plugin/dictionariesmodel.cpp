/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "dictionariesmodel.h"
#include <Plasma/DataContainer>
#include <QDebug>

DictionariesModel::DictionariesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    Plasma::DataEngine *dataengine = dataEngine(QStringLiteral("dict"));
    const QString source = QLatin1String("list-dictionaries");
    Plasma::DataContainer *container = dataengine->containerForSource(source);
    if (container) { // in practice this never seems to happen, this source is only used here, so never shared
        setAvailableDicts(container->data());
    }
    dataengine->connectSource(source, this);
}

QVariant DictionariesModel::data(const QModelIndex &index, int role) const
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

int DictionariesModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid())
        return 0; // flat model
    return m_availableDicts.size();
}

QHash<int, QByteArray> DictionariesModel::roleNames() const
{
    return {{Qt::DisplayRole, "description"}, {Qt::EditRole, "id"}};
}

void DictionariesModel::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_ASSERT(sourceName == QLatin1String("list-dictionaries"));
    beginResetModel();
    setAvailableDicts(data);
    endResetModel();
}

void DictionariesModel::setAvailableDicts(const QVariantMap &data)
{
    m_availableDicts = {};
    m_availableDicts.resize(data.count());
    int i = 0;
    for (auto it = data.begin(), end = data.end(); it != end; ++it, ++i) {
        m_availableDicts[i] = AvailableDict{it.key(), it.value().toString()};
    }
}
