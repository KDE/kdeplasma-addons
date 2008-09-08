/*
*   Copyright (C) 2008 Hugo Parente Lima <hugo.pl@gmail.com>
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

#include "malvadosprovider.h"

#include <QtGui/QImage>

#define MALVADOS_URL "http://www.malvados.com.br/"
#define MALVADOS_COMIC_IMAGE "tirinha%1.gif"
#define MALVADOS_COMIC_HTML "index%1.html"

COMICPROVIDER_EXPORT_PLUGIN( MalvadosProvider, "MalvadosProvider", "" )

class MalvadosProvider::Private
{
    public:

        enum RequestType
        {
            PageRequest, // requesting index.html (a frameset).
            PageSubRequest, // requesting the html in main frame.
            ImageRequest, // requesting the comic.
            CheckNextRequest // requesting the html of the next comic, just to check if it exists.
        };

        Private()
            : mComicId( 0 ), mHasNext( false )
        {
        }

        QImage mImage;
        int mComicId;
        bool mHasNext;
};

MalvadosProvider::MalvadosProvider( QObject* parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    d->mComicId = requestedNumber();
    KUrl url( MALVADOS_URL );

    if ( d->mComicId > 0 ) {
        url.setPath( QString( MALVADOS_COMIC_IMAGE ).arg( d->mComicId ) );
        requestPage( url, Private::ImageRequest );
    } else {
        requestPage( url, Private::PageRequest );
    }
}

MalvadosProvider::~MalvadosProvider()
{
    delete d;
}

ComicProvider::IdentifierType MalvadosProvider::identifierType() const
{
    return NumberIdentifier;
}

KUrl MalvadosProvider::websiteUrl() const
{
    return KUrl( MALVADOS_URL );
}

QImage MalvadosProvider::image() const
{
    return d->mImage;
}

QString MalvadosProvider::identifier() const
{
    return QString( "malvados:%1" ).arg( requestedNumber() );
}

QString MalvadosProvider::nextIdentifier() const
{
    return d->mHasNext ? QString::number( d->mComicId + 1 ) : QString();
}

QString MalvadosProvider::previousIdentifier() const
{
    return QString::number( d->mComicId - 1 );
}

void MalvadosProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    KUrl url( MALVADOS_URL );

    if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        if ( requestedNumber() > 0 ) {
            // ok, we have the comic, let check if the next comic exists
            url.setPath( QString( MALVADOS_COMIC_HTML ).arg( d->mComicId + 1 ) );
            requestPage( url, Private::CheckNextRequest );
        } else {
            emit finished( this );
        }
    } else {
        QRegExp regex( "index(\\d+)\\.html\"" );
        const QString data = QString::fromLatin1( rawData );
        bool comicFound = regex.indexIn( data ) >= 0;

        if ( id == Private::CheckNextRequest ) {
            d->mHasNext = comicFound;
            emit finished( this );
        } else {
            if ( comicFound ) {
                d->mComicId = regex.cap( 1 ).toInt();
                url.setPath( QString( MALVADOS_COMIC_IMAGE ).arg( d->mComicId ) );
                requestPage( url, Private::ImageRequest );
            } else if ( id != Private::PageSubRequest ) {
                // Andre Dahmer is drunk, so it put something in the main page.
                QRegExp otherIndexRegex( "name=\"mainFrame\"\\s+src=\"([\\w\\.]+)\"", Qt::CaseInsensitive );
                otherIndexRegex.indexIn( data );
                url.setPath( otherIndexRegex.cap( 1 ) );
                requestPage( url, Private::PageSubRequest );
            } else {
                emit error( this );
            }
        }
    }
}

void MalvadosProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "malvadosprovider.moc"
