/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicmodel.h"
#include "types.h"

#include <QDebug>
#include <QIcon>
#include <QList>

ComicModel::ComicModel(ComicEngine *engine, const QStringList &enabledProviders, QObject *parent)
    : QAbstractTableModel(parent)
    , mEnabledProviders(enabledProviders)
    , mEngine(engine)
{
    Q_ASSERT(engine);

    load();
}

QHash<int, QByteArray> ComicModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Qt::CheckStateRole] = "checked";
    roles[ComicPluginRole] = "plugin";
    roles[ComicEnabledRole] = "enabled";
    roles[ComicHighlightRole] = "highlight";
    return roles;
}

int ComicModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return mComics.count();
    }

    return 0;
}

int ComicModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return 2;
}

QVariant ComicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mComics.count()) {
        return QVariant();
    }

    const ComicProviderInfo &info = mComics.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return info.name;
    case Qt::DecorationRole:
        return info.icon;
    case Qt::CheckStateRole:
        return mChecked[index.row()];
    case ComicPluginRole:
        return info.pluginId;
    case ComicEnabledRole:
        return mEnabled[index.row()];
    case ComicHighlightRole:
        return mHighlighted[index.row()];
    }

    return QVariant();
}

Qt::ItemFlags ComicModel::flags(const QModelIndex &index) const
{
    if (index.isValid() && (index.column() == 0)) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool ComicModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole) {
        mChecked[index.row()] = value.toBool();
        Q_EMIT dataChanged(index, index, {role});
        return true;
    }
    return false;
}

void ComicModel::load()
{
    beginResetModel();
    mComics = mEngine->loadProviders();
    mEnabled = QBitArray(mComics.count());
    mChecked = QBitArray(mComics.count());
    mHighlighted = QBitArray(mComics.count());
    for (int i = 0; i < mComics.count(); ++i) {
        mChecked[i] = mEnabledProviders.contains(mComics.at(i).pluginId);
        mEnabled[i] = mEnabledProviders.contains(mComics.at(i).pluginId);
    }
    endResetModel();
}

QStringList ComicModel::checkedProviders()
{
    QStringList checkedProviders;
    checkedProviders.reserve(mChecked.count(true));
    for (int i = 0; i < mComics.count(); ++i) {
        if (mChecked[i]) {
            checkedProviders << mComics.at(i).pluginId;
        }
    }
    checkedProviders.sort();
    return checkedProviders;
}

QStringList ComicModel::enabledProviders() const
{
    return mEnabledProviders;
}

void ComicModel::setEnabledProviders(const QStringList &enabledProviders)
{
    mEnabledProviders = enabledProviders;
    load();
}

void ComicModel::setHighlight(const QModelIndex &index, bool highlight)
{
    mHighlighted[index.row()] = highlight;
    Q_EMIT dataChanged(index, index, {ComicHighlightRole});
}
