/*
*   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include <QtCore/QRegExp>
#include <QtGui/QImage>

#include <KUrl>

#include "phdprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( PhdProvider, "PhdProvider", "" )

class PhdProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };

        Private()
        : mHasNextComic( false )
        {
        }

        QImage mImage;
        bool mHasNextComic;
        int mRequestedId;
};


PhdProvider::PhdProvider( QObject *parent, const QVariantList &args )
: ComicProvider( parent, args ), d( new Private )
{
    d->mRequestedId = requestedNumber();

    KUrl baseUrl( QString( "http://www.phdcomics.com/" ) );

    if ( d->mRequestedId > 0 ) {
        baseUrl.setPath( "/comics/archive.php" );
        baseUrl.addQueryItem( "comicid", QString::number( d->mRequestedId ) );
    } else {
        baseUrl.setPath( "/comics/archive.php" );
    }

    requestPage( baseUrl, Private::PageRequest );
}

PhdProvider::~PhdProvider()
{
    delete d;
}

ComicProvider::IdentifierType PhdProvider::identifierType() const
{
    return NumberIdentifier;
}

QImage PhdProvider::image() const
{
    return d->mImage;
}

QString PhdProvider::identifier() const
{
    return QString( "phd:%1" ).arg( d->mRequestedId );
}

KUrl PhdProvider::websiteUrl() const
{
    return QString( "http://www.phdcomics.com/comics/archive.php?comicid=%1" ).arg( d->mRequestedId );
}

QString PhdProvider::nextIdentifier() const
{
    if ( d->mHasNextComic ) {
        return QString::number( d->mRequestedId + 1 );
    } else {
        return QString();
    }
}

QString PhdProvider::previousIdentifier() const
{
    if ( d->mRequestedId > 1 ) {
        return QString::number( d->mRequestedId - 1 );
    } else {
        return QString();
    }
}

void PhdProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString pattern( "\\<img src=http://www\\.phdcomics\\.com/comics/archive/" );
        const QRegExp exp( pattern );

        const QString data = QString::fromUtf8( rawData );

        const int pos = exp.indexIn( data ) + pattern.length();
        const QString sub = data.mid( pos, data.indexOf( ' ', pos ) - pos );

        KUrl url( QString( "http://www.phdcomics.com/comics/archive/phd%1" ).arg( sub ) );

        requestPage( url, Private::ImageRequest );

        // search the id of this comic if it was not specified
        if ( d->mRequestedId < 1 ) {
            const QString idPattern( "tellafriend\\.php\\?comicid=(\\d+)" );
            QRegExp idExp( idPattern );

            if ( idExp.indexIn( data ) > -1 ) {
                d->mRequestedId = idExp.cap( 1 ).toInt();
            }
        }

        // now search if there is a next comic
        const QString nextPattern( "images/next_button.gif" );
        const QRegExp nextExp( nextPattern );

        d->mHasNextComic = (nextExp.indexIn( data ) != -1);
    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void PhdProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "phdprovider.moc"
