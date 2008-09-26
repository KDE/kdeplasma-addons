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

#include "pluginmanager.h"

#include <KServiceTypeTrader>
#include <KStandardDirs>

ComicEntry::ComicEntry( const QString &_identifier, const QString &_title,
                        const QString &_suffixType, const QPixmap &_icon )
    : identifier( _identifier ), title( _title ), suffixType( _suffixType ), icon( _icon )
{
}

bool ComicEntry::operator<( const ComicEntry &other ) const
{
    return ( title < other.title );
}

PluginManager::PluginManager()
{
    KService::List services = KServiceTypeTrader::self()->query( "PlasmaComic/Plugin" );
    Q_FOREACH ( const KService::Ptr &service, services ) {
        mComics << ComicEntry( service->property( "X-KDE-PlasmaComicProvider-Identifier", QVariant::String ).toString(),
                                service->name(),
                                service->property( "X-KDE-PlasmaComicProvider-SuffixType", QVariant::String ).toString(),
                                QPixmap( KStandardDirs::locate( "data", QString( "plasma-comic/%1.png" ).arg( service->icon() ) ) ) );
    }

    qSort( mComics );

    for ( int i = 0; i < mComics.size(); ++i ) {
        map.insert( mComics.at( i ).identifier, i );
    }
}

PluginManager* PluginManager::Instance()
{
    static PluginManager instance;
    return &instance;
}

QList<ComicEntry> PluginManager::comics() const
{
    return mComics;
}

QString PluginManager::comicTitle( const QString &identifier ) const
{
    int index =  map[ identifier ];
    return mComics.at( index ).title;
}

QString PluginManager::suffixType( const QString &identifier ) const
{
    int index = map[ identifier ];
    return mComics.at( index ).suffixType;
}
