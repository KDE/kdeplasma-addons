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
#include <KDebug>

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
    : Plasma::DataEngine( parent ), mEmptySuffix(false)
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
    const QStringList parts = identifier.split( ':', QString::KeepEmptyParts );

    //: are mandatory
    if (parts.count() < 2) {
        return false;
    }

    ComicProvider *provider = 0;

    ComicProvider::SuffixType st = ComicProvider::suffixType(parts[ 0 ]);

    //Here goes who uses dates
    if (st == ComicProvider::DateSuffix) {
        QDate date = QDate::fromString( parts[ 1 ], Qt::ISODate );

        //default is today ()
        if (!date.isValid() || date.isNull()) {
            date = QDate::currentDate();
            mEmptySuffix = true;
        } else {
            mEmptySuffix = false;
        }

        if (parts[ 0 ] == "userfriendly") {
            provider = new UserFriendlyProvider( date, this );
        } else if (parts[ 0 ] == "dilbert") {
            provider = new DilbertProvider( date, this );
        } else if (parts[ 0 ] == "garfield") {
            provider = new GarfieldProvider( date, this );
        } else if (parts[ 0 ] == "snoopy") {
            provider = new SnoopyProvider( date, this );
        } else if (parts[ 0 ] == "osnews") {
            provider = new OsNewsProvider( date, this );
        //Invalid name asked
        } else {
            return false;
        }

    //Here goes who use int ids
    } else if (st == ComicProvider::IntSuffix) {
        bool ok;
        int requestedId = parts[ 1 ].toInt(&ok);

        if (!ok) {
            requestedId = -1;
            mEmptySuffix = true;
        } else {
            mEmptySuffix = false;
        }

        if (parts[ 0 ] == "xkcd") {
            provider = new XkcdProvider( requestedId, this );
        //Invalid name asked
        } else {
            return false;
        }
    //No other types supported at the moment
    } else {
        return false;
    }


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
    //if it was asked an empty suffix return an empty suffix
    if (mEmptySuffix) {
        identifier = identifier.left(identifier.indexOf(':') + 1);
    }

    setData( identifier, "Image", provider->image() );
    setData( identifier, "Website Url", provider->websiteUrl());
    setData( identifier, "Next identifier suffix", provider->nextIdentifierSuffix());
    setData( identifier, "Previous identifier suffix", provider->previousIdentifierSuffix());

    // store in cache if it's not the response of a CachedProvider,
    // if there is a valid image and if there is a next comic
    // (if we're on today's comic it could become stale)
    if ( dynamic_cast<CachedProvider*>( provider ) == 0 && !provider->image().isNull() && !provider->nextIdentifierSuffix().isNull() ) {
        CachedProvider::Settings info;

        info["websiteUrl"] = provider->websiteUrl().prettyUrl();
        info["nextIdentifierSuffix"] = provider->nextIdentifierSuffix();
        info["previousIdentifierSuffix"] = provider->previousIdentifierSuffix();

        CachedProvider::storeInCache( provider->identifier(), provider->image(), info );
    }

    provider->deleteLater();
}

void ComicEngine::error( ComicProvider *provider )
{
    setData( provider->identifier(), QImage() );

    provider->deleteLater();
}

#include "comic.moc"
