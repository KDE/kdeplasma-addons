/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "dictionariesmodel.h"
#include <QDebug>

DictionariesModel::DictionariesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    static DictEngine engine; // Keep this around longer, because then we can use it's cache'
    connect(&engine, &DictEngine::dictErrorOccurred, this, &DictionariesModel::slotDictErrorOccurred);
    connect(&engine, &DictEngine::dictsRecieved, this, [this](const QMap<QString, QString> &dicts) {
        beginResetModel();
        m_availableDicts = {};
        m_availableDicts.resize(dicts.count());
        int i = 0;
        for (auto it = dicts.begin(), end = dicts.end(); it != end; ++it, ++i) {
            m_availableDicts[i] = AvailableDict{it.key(), it.value()};
        }
        endResetModel();
    });
    connect(&engine, &DictEngine::dictLoadingChanged, this, &DictionariesModel::slotDictLoadingChanged);

    engine.requestDicts();
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
    if (index.isValid()) {
        return 0; // flat model
    }
    return m_availableDicts.size();
}

QHash<int, QByteArray> DictionariesModel::roleNames() const
{
    return {{Qt::DisplayRole, "description"}, {Qt::EditRole, "id"}};
}

bool DictionariesModel::loading() const
{
    return m_loading;
}

QAbstractSocket::SocketError DictionariesModel::errorCode() const
{
    return m_errorCode;
}

QString DictionariesModel::errorString() const
{
    return m_errorString;
}

void DictionariesModel::slotDictErrorOccurred(QAbstractSocket::SocketError socketError, const QString &errorString)
{
    m_errorCode = socketError;
    m_errorString = errorString;

    Q_EMIT errorCodeChanged();
    Q_EMIT errorStringChanged();
}

void DictionariesModel::slotDictLoadingChanged(bool loading)
{
    m_loading = loading;

    Q_EMIT loadingChanged();
}
