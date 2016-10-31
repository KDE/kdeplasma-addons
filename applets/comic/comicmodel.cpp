/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
 *   Copyright (C) 2015 Marco Martin <mart@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "comicmodel.h"

#include <QIcon>
#include <QDebug>

ComicModel::ComicModel( Plasma::DataEngine *engine, const QString &source, const QStringList &usedComics, QObject *parent )
  : QAbstractTableModel( parent ), mUsedComics(usedComics)
{
    if (engine) {
        engine->connectSource( source, this );
    }
}

void ComicModel::dataUpdated( const QString &/*source*/, const Plasma::DataEngine::Data &data )
{
    setComics( data, mUsedComics );
}

QHash<int, QByteArray> ComicModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Qt::UserRole] = "plugin";
    return roles;
}

void ComicModel::setComics( const Plasma::DataEngine::Data &comics, const QStringList &/*usedComics*/ )
{
    beginResetModel();

    mComics = comics;

    endResetModel();
}

int ComicModel::rowCount( const QModelIndex &index ) const
{
    if ( !index.isValid() ) {
        return mComics.count();
    }

    return 0;
}

int ComicModel::columnCount( const QModelIndex &index ) const
{
    Q_UNUSED( index )
    return 2;
}

QVariant ComicModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() || index.row() >= mComics.keys().count() ) {
        return QVariant();
    }

    const QString data = mComics.keys()[ index.row() ];

    switch( role ) {
        case Qt::DisplayRole:
            return mComics[ data ].toStringList()[ 0 ];
        case Qt::DecorationRole:
            return QIcon::fromTheme( mComics[ data ].toStringList()[ 1 ] );
        case Qt::UserRole:
            return data;
    }


    return QVariant();
}

Qt::ItemFlags ComicModel::flags( const QModelIndex &index ) const
{
    if ( index.isValid() && ( index.column() == 0 ) ) {
        return QAbstractItemModel::flags( index ) | Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


