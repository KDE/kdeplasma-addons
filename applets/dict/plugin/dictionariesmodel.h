/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef DICTIONARIES_MODEL_H
#define DICTIONARIES_MODEL_H

#include "../../dict/dictengine.h"
#include <QAbstractListModel>
#include <vector>

class DictionariesModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @return the number of dict items
     */
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    /**
     * @return @c true if the engine is downloading dict list from
     * the Internet, @c false otherwise.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    explicit DictionariesModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;
    bool loading() const;

Q_SIGNALS:
    void countChanged();
    void loadingChanged();

private:
    void setAvailableDicts(const QVariantMap &data);

    struct AvailableDict {
        QString id;
        QString description;
    };
    std::vector<AvailableDict> m_availableDicts;

    bool m_loading = false;
};

#endif
