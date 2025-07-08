/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "util.h"

#include <KUnitConversion/Unit>

#include <QAbstractListModel>
#include <QList>

class UnitItem
{
public:
    UnitItem()
    {
    }
    UnitItem(KUnitConversion::UnitId _unitId)
        : name(Util::nameFromUnitId(_unitId))
        , unitId(_unitId)
    {
    }

    QString name;
    KUnitConversion::UnitId unitId;
};

Q_DECLARE_METATYPE(UnitItem)
Q_DECLARE_TYPEINFO(UnitItem, Q_MOVABLE_TYPE);

class AbstractUnitListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int defaultUnit READ defaultUnitId CONSTANT)

public:
    explicit AbstractUnitListModel(const QList<UnitItem> &items, int defaultUnitId, QObject *parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;

public:
    Q_INVOKABLE int listIndexForUnitId(int unitId) const;
    Q_INVOKABLE int unitIdForListIndex(int listIndex) const;
    int defaultUnitId() const;

private:
    const QList<UnitItem> m_items;
    const int m_defaultIndex;
};
