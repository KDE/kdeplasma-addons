/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtCore/QDate>

#include <KUrl>

#include "comic.h"

// comic providers
#include "cachedprovider.h"
#include "dilbertprovider.h"
#include "garfieldprovider.h"
#include "snoopyprovider.h"
#include "userfriendlyprovider.h"
#include "xkcdprovider.h"
#include "osnewsprovider.h"

ComicEngine::ComicEngine( QObject* parent, const QVariantList& )
    : Plasma::DataEngine( parent )
{
    setUpdateInterval( 0 );
}

ComicEngine::~ComicEngine()
{
}

void ComicEngine::init()
{
}

bool ComicEngine::updateSource( const QString &identifier )
{
    // check whether it is cached already...
    if ( CachedProvider::isCached( identifier ) ) {
        ComicProvider *provider = new CachedProvider( identifier, this );
        connect( provider, SIGNAL( finished( ComicProvider* ) ), this, SLOT( finished( ComicProvider* ) ) );
        connect( provider, SIGNAL( error( ComicProvider* ) ), this, SLOT( error( ComicProvider* ) ) );
        return true;
    }

    // ... start a new query otherwise
    const QStringList parts = identifier.split( ':', QString::SkipEmptyParts );

    QDate date = QDate::fromString( parts[ 1 ], Qt::ISODate );
    if ( !date.isValid() ) {
        return false;
    }

    ComicProvider *provider = 0;
    if ( parts[ 0 ] == "userfriendly" )
      provider = new UserFriendlyProvider( date, this );
    else if ( parts[ 0 ] == "dilbert" )
      provider = new DilbertProvider( date, this );
    else if ( parts[ 0 ] == "garfield" )
      provider = new GarfieldProvider( date, this );
    else if ( parts[ 0 ] == "snoopy" )
      provider = new SnoopyProvider( date, this );
    else if ( parts[ 0 ] == "xkcd" )
      provider = new XkcdProvider( date, this );
    else if ( parts[ 0 ] == "osnews" )
      provider = new OsNewsProvider( date, this );

    connect( provider, SIGNAL( finished( ComicProvider* ) ), this, SLOT( finished( ComicProvider* ) ) );
    connect( provider, SIGNAL( error( ComicProvider* ) ), this, SLOT( error( ComicProvider* ) ) );

    return true;
}

bool ComicEngine::sourceRequested( const QString &identifier )
{
    setData( identifier, DataEngine::Data() );

    return updateSource( identifier );
}

void ComicEngine::finished( ComicProvider *provider )
{
    QString identifier(provider->identifier());
    setData( identifier, "image", provider->image() );

    // store in cache if it's not the response of a CachedProvider
    if ( dynamic_cast<CachedProvider*>( provider ) == 0 && !provider->image().isNull() ) {
        CachedProvider::Settings info;
        info["websiteUrl"] = provider->websiteUrl().prettyUrl();
        CachedProvider::storeInCache( provider->identifier(), provider->image(), info );
    }

    setData( identifier, "websiteUrl", provider->websiteUrl());

    provider->deleteLater();
}

void ComicEngine::error( ComicProvider *provider )
{
    setData( provider->identifier(), QImage() );

    provider->deleteLater();
}

#include "comic.moc"
