/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
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

#include <KIcon>

ComicModel::ComicModel( const Plasma::DataEngine::Data &comics, const QStringList &usedComics, QObject *parent )
  : QAbstractTableModel( parent ), mNumSelected( 0 )
{
    setComics( comics, usedComics );
}

void ComicModel::setComics( const Plasma::DataEngine::Data &comics, const QStringList &usedComics )
{
    beginResetModel();

    mNumSelected = 0;
    mComics = comics;
    mState.clear();
    Plasma::DataEngine::Data::const_iterator it;
    Plasma::DataEngine::Data::const_iterator itEnd = mComics.constEnd();
    for ( it = mComics.constBegin(); it != itEnd; ++it ) {
        const bool isChecked = usedComics.contains( it.key() );
        mState[ it.key() ] = ( isChecked ? Qt::Checked : Qt::Unchecked );
        if ( isChecked ) {
            ++mNumSelected;
        }
    }

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
    if ( index.column() == 0 ) {
        if ( role == Qt::CheckStateRole ) {
            return mState[ data ];
        }
    } else if ( index.column() == 1 ) {
        switch( role ) {
            case Qt::DisplayRole:
                return mComics[ data ].toStringList()[ 0 ];
            case Qt::DecorationRole:
                return KIcon( mComics[ data ].toStringList()[ 1 ] );
            case Qt::UserRole:
                return data;
        }
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

bool ComicModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( index.isValid() && ( role == Qt::CheckStateRole ) ) {
        Qt::CheckState oldState = mState[ mComics.keys()[ index.row() ] ];
        Qt::CheckState newState = static_cast< Qt::CheckState >( value.toInt() );
        mState[ mComics.keys()[ index.row() ] ] = newState;
        if ( newState != oldState ) {
            if ( newState == Qt::Checked ) {
                ++mNumSelected;
            } else if ( newState == Qt::Unchecked ) {
                --mNumSelected;
            }
        }
        emit dataChanged( index, index );
        return true;
    }

    return false;
}

int ComicModel::numSelected() const
{
    return mNumSelected;
}

QStringList ComicModel::selected() const
{
    QStringList list;
    QHash< QString, Qt::CheckState >::const_iterator it;
    QHash< QString, Qt::CheckState >::const_iterator itEnd = mState.constEnd();
    for ( it = mState.constBegin(); it != itEnd; ++it ) {
        if ( it.value() == Qt::Checked ) {
            list << it.key();
        }
    }

    return list;
}
