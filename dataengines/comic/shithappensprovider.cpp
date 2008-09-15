/*
*   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
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

#include <QtCore/QRegExp>
#include <QtGui/QPainter>
#include <QtGui/QImage>

#include <KUrl>
#include <KStandardDirs>

#include "shithappensprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( ShitHappensProvider, "ShitHappensProvider", "" )

class ShitHappensProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };
        Private()
            : mPreviousId ( 0 ), mNextId ( 0 )
        {
        }

        QImage mImage;
        int mRequestedId;
        int mPreviousId;
        int mNextId;
};


ShitHappensProvider::ShitHappensProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    d->mRequestedId = requestedNumber();

    KUrl url( QString( "http://www.ruthe.de/" ) );

    if ( d->mRequestedId ) {
        url.setPath( "/frontend/index.php" );
        url.addQueryItem( "pic", QString::number( d->mRequestedId ) );
    }
    else {
        url.setPath( "/frontend/" );
    }

    requestPage( url, Private::PageRequest );
}

ShitHappensProvider::~ShitHappensProvider()
{
    delete d;
}

ComicProvider::IdentifierType ShitHappensProvider::identifierType() const
{
    return NumberIdentifier;
}

QImage ShitHappensProvider::image() const
{
    return d->mImage;
}

QString ShitHappensProvider::identifier() const
{
    return QString( "shithappens:%1" ).arg( d->mRequestedId );
}

KUrl ShitHappensProvider::websiteUrl() const
{
    return QString( "http://www.ruthe.de/frontend/index.php?pic=%1&sort=name&order=DESC" )
    .arg( d->mRequestedId );
}

QString ShitHappensProvider::nextIdentifier() const
{
    if ( d->mNextId > 1 ) {
        return QString::number( d->mNextId );
    } else {
        return QString();
    }
}

QString ShitHappensProvider::previousIdentifier() const
{
    if ( ( d->mPreviousId > 0 ) ) {
        // no Previous ID for the first comic
        if ( ( d->mNextId == 0 ) || ( d->mPreviousId < d->mNextId ) ) {
            return QString::number( d->mPreviousId );
        }
    }

    return QString();
}

void ShitHappensProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString data = QString::fromUtf8( rawData );

        const QString pattern( "<img src=\"(cartoons/strip_\\d+\\.jpg)\"" );
        QRegExp exp( pattern );
        const int pos = exp.indexIn( data );

        KUrl url;

        if ( pos > -1 ) {
            url = KUrl( QString( "http://www.ruthe.de/frontend/%1"  ).arg( exp.cap( 1 ) ) );
            // get the current id
            if ( !d->mRequestedId ) {
                const QString patternId( "<a href=\"archiv.php\\?sort=name&order=DESC&id=(\\d+)\".*\"><img src=\"bilder/arch.gif\"" );
                QRegExp expId( patternId );
                const int posId = expId.indexIn( data );

                if ( posId > -1 ) {
                    d->mRequestedId = expId.cap( 1 ).toInt();
                }
            }
        }

        // get previous id
        const QString patternPrevious( "<a href=\"index.php\\?pic=(\\d+)&sort=name&order=DESC\".*\"><img src=\"bilder/back.gif\"" );
        QRegExp expPrevious ( patternPrevious );
        const int posPrevious = expPrevious.indexIn( data );
        if ( posPrevious > -1 ) {
            d->mPreviousId = expPrevious.cap( 1 ).toInt();
        }

        // get next id
        const QString patternNext( "<div align=\"right\"><a href=\"index.php\\?pic=(\\d+)&sort=name&order=DESC\".*\"><img src=\"bilder/weiter.gif\"" );
        QRegExp expNext ( patternNext );
        const int posNext = expNext.indexIn( data );
        if ( posNext > -1 ) {
            d->mNextId = expNext.cap( 1 ).toInt();
        }

        requestPage( url, Private::ImageRequest );
    } else if ( id == Private::ImageRequest ) {
        const QString headerRelLoc( "plasma-comic/plasma_comic_shithappens-header.png" );
        const QImage header( KStandardDirs::locate( "data", headerRelLoc ) );
        const QImage comic = QImage::fromData( rawData );

        const int spaceTop = 8;
        const int spaceMid = 20;
        const int spaceBot = 13;

        const int height = header.height() + comic.height() + spaceTop + spaceMid + spaceBot;
        const int width = (header.width() >= comic.width()) ? header.width() : comic.width();

        d->mImage = QImage( QSize( width, height ), QImage::Format_RGB32 );
        d->mImage.fill( header.pixel( QPoint( 0, 0 ) ) );

        QPainter painter( &(d->mImage) );

        // center and draw the Images
        const QPoint headerPos( ( ( width - header.width() ) / 2 ), spaceTop );
        const QPoint comicPos( ( ( width - comic.width() ) / 2 ), spaceTop + header.height() + spaceMid );
        painter.drawImage( headerPos, header );
        painter.drawImage( comicPos, comic );

        emit finished( this );
    }
}

void ShitHappensProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "shithappensprovider.moc"
