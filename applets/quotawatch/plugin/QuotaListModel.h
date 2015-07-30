/*
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
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
#ifndef PLASMA_QUOTA_LIST_MODEL_H
#define PLASMA_QUOTA_LIST_MODEL_H

#include <QAbstractListModel>
#include <QVector>

#include "quotaitem.h"

/**
 * Class monitoring the file system quota.
 * The monitoring is performed through a timer, running the 'quota'
 * command line tool.
 */
class QuotaListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    QuotaListModel(QObject * parent = nullptr);

    /**
     * List of available roles for the QML ListView.
     */
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    /**
     * Returns the data for @p index and given @p role.
     */
    QVariant data(const QModelIndex & index, int role) const Q_DECL_OVERRIDE;

    /**
     * Returns the number of items for the toplevel model index, otherwise 0.
     */
    int rowCount(const QModelIndex & index) const Q_DECL_OVERRIDE;

    /**
     * Changes the data for @p index to @p variant.
     */
    bool setData(const QModelIndex & index, const QVariant & variant, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;

    /**
     *
     */
    void updateItems(const QVector<QuotaItem> & items);

    /**
     * Clears all items in the model.
     */
    void clear();

private:
    QVector<QuotaItem> m_items;
};

#endif // PLASMA_QUOTA_LIST_MODEL_H
