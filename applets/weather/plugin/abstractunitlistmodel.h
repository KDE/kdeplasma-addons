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

#ifndef ABSTRACTUNITLISTMODEL_H
#define ABSTRACTUNITLISTMODEL_H

#include "util.h"

#include <KUnitConversion/Unit>

#include <QAbstractListModel>
#include <QVector>

class UnitItem
{
public:
    UnitItem() {}
    UnitItem(KUnitConversion::UnitId _unitId) : name(Util::nameFromUnitId(_unitId)), unitId(_unitId) {}

    QString name;
    KUnitConversion::UnitId unitId;
};

Q_DECLARE_METATYPE(UnitItem)
Q_DECLARE_TYPEINFO(UnitItem, Q_MOVABLE_TYPE);


class AbstractUnitListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AbstractUnitListModel(const QVector<UnitItem> &items, QObject *parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;

public:
    Q_INVOKABLE int listIndexForUnitId(int unitId) const;
    Q_INVOKABLE int unitIdForListIndex(int listIndex) const;

private:
    const QVector<UnitItem> m_items;
};

#endif // ABSTRACTUNITLISTMODEL_H
