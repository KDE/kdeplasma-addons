/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
