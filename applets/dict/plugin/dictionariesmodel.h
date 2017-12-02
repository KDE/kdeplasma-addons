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
#ifndef DICTIONARIES_MODEL_H
#define DICTIONARIES_MODEL_H

#include <QAbstractListModel>
#include <Plasma/DataEngineConsumer>
#include <Plasma/DataEngine>
#include <vector>

class DictionariesModel : public QAbstractListModel, public Plasma::DataEngineConsumer
{
    Q_OBJECT

public:
    explicit DictionariesModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

private:
    void setAvailableDicts(const QVariantMap &data);

    struct AvailableDict {
        QString id;
        QString description;
    };
    std::vector<AvailableDict> m_availableDicts;
};

#endif
