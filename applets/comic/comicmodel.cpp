/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicmodel.h"

#include <QDebug>
#include <QIcon>

ComicModel::ComicModel(Plasma::DataEngine *engine, const QString &source, const QStringList &usedComics, QObject *parent)
    : QAbstractTableModel(parent)
    , mUsedComics(usedComics)
{
    if (engine) {
        engine->connectSource(source, this);
    }
}

void ComicModel::dataUpdated(const QString & /*source*/, const Plasma::DataEngine::Data &data)
{
    setComics(data, mUsedComics);
}

QHash<int, QByteArray> ComicModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Qt::UserRole] = "plugin";
    return roles;
}

void ComicModel::setComics(const Plasma::DataEngine::Data &comics, const QStringList & /*usedComics*/)
{
    beginResetModel();

    mComics = comics;

    endResetModel();
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
    if (!index.isValid() || index.row() >= mComics.keys().count()) {
        return QVariant();
    }

    const QString data = mComics.keys()[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return mComics[data].toStringList()[0];
    case Qt::DecorationRole:
        return QIcon::fromTheme(mComics[data].toStringList()[1]);
    case Qt::UserRole:
        return data;
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
