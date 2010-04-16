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

#ifndef COMICMODEL_H
#define COMICMODEL_H

#include <QtCore/QAbstractTableModel>

#include <Plasma/DataEngine>

class ComicModel : public QAbstractTableModel
{
    public:
        ComicModel( const Plasma::DataEngine::Data &comics, const QStringList &usedComics, QObject *parent = 0 );

        void setComics( const Plasma::DataEngine::Data &comics, const QStringList &usedComics );

        int rowCount( const QModelIndex &index = QModelIndex() ) const;
        int columnCount( const QModelIndex &index = QModelIndex() ) const;
        QVariant data( const QModelIndex &index, int role = Qt::CheckStateRole ) const;
        Qt::ItemFlags flags( const QModelIndex &index ) const;
        bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

        int numSelected() const;
        QStringList selected() const;

    private:
        Plasma::DataEngine::Data mComics;
        QHash<QString, Qt::CheckState> mState;
        int mNumSelected;
};

#endif
