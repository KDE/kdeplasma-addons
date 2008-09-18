/***************************************************************************
*   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
*   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtGui/QPixmap>
#include <QtCore/QMap>

class ComicEntry
{
    public:
        ComicEntry( const QString &_identifier, const QString &_title, const QPixmap &_icon );

        bool operator<( const ComicEntry &other ) const;

        QString identifier;
        QString title;
        QPixmap icon;
};

class PluginManager
{
    public:
        ~PluginManager(){}
        static PluginManager* Instance();

        QList<ComicEntry> comics() const;
        QString comicTitle( const QString &identifier ) const;

    private:
        PluginManager();
        PluginManager(const PluginManager& rhs);

        QList<ComicEntry> mComics;
        QMap<QString, int> map;
};

#endif
