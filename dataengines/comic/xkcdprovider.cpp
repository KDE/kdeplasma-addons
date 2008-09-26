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

#include <QtCore/QRegExp>
#include <QtGui/QImage>

#include <KUrl>

#include "xkcdprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( XkcdProvider, "XkcdProvider", "" )

class XkcdProvider::Private
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
        QString mStripTitle;
        QString mAdditionalText;
        bool mHasNextComic;
        int mRequestedId;
};


XkcdProvider::XkcdProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setComicAuthor( "Randall Munroe" );
    d->mRequestedId = requestedNumber();

    KUrl baseUrl( QString( "http://xkcd.com/" ) );

    if ( d->mRequestedId > 0 ) {
        baseUrl.setPath( QString::number( d->mRequestedId ) + '/' );
    }

    requestPage( baseUrl, Private::PageRequest );
}

XkcdProvider::~XkcdProvider()
{
    delete d;
}

ComicProvider::IdentifierType XkcdProvider::identifierType() const
{
    return NumberIdentifier;
}

QImage XkcdProvider::image() const
{
    return d->mImage;
}

QString XkcdProvider::identifier() const
{
    return QString( "xkcd:%1" ).arg( d->mRequestedId );
}

KUrl XkcdProvider::websiteUrl() const
{
    return QString( "http://xkcd.com/%1/" ).arg( d->mRequestedId );
}

QString XkcdProvider::nextIdentifier() const
{
   if ( d->mHasNextComic ) {
       return QString::number( d->mRequestedId + 1 );
   } else {
       return QString();
   }
}

QString XkcdProvider::previousIdentifier() const
{
   if ( d->mRequestedId > 1 ) {
       return QString::number( d->mRequestedId - 1 );
   } else {
       return QString();
   }
}

QString XkcdProvider::stripTitle() const
{
    return d->mStripTitle;
}

QString XkcdProvider::additionalText() const
{
    return d->mAdditionalText;
}

void XkcdProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString pattern( "<img src=\"http://imgs.xkcd.com/comics/" );
        const QRegExp exp( pattern );

        const QString data = QString::fromUtf8( rawData );

        const int pos = exp.indexIn( data ) + pattern.length();
        const QString sub = data.mid( pos, data.indexOf( '.', pos ) - pos + 4 );

        KUrl url( QString( "http://imgs.xkcd.com/comics/%1" ).arg( sub ) );

        requestPage( url, Private::ImageRequest );

        // search the id of this comic if it was not specified
        if ( d->mRequestedId < 1 ) {
            const QString idPattern( "http://xkcd.com/(\\d+)/" );
            QRegExp idExp( idPattern );

            if ( idExp.indexIn( data ) > -1 ) {
                d->mRequestedId = idExp.cap( 1 ).toInt();
            }
        }


        // now search if there is a next comic
        const QString nextPattern( "href=\"#\" accesskey=\"n\">" );
        const QRegExp nextExp( nextPattern );

        d->mHasNextComic = ( nextExp.indexIn( data ) == -1 );

        //find the tooltip and the strip title of the comic
        const QString toolStripPattern( "src=\"http://imgs.xkcd.com/comics/.+\" title=\"(.+)\" alt=\"(.+)\"" );
        QRegExp toolStripExp ( toolStripPattern );
        toolStripExp.setMinimal( true );
        if ( toolStripExp.indexIn ( data ) > 1 ) {
            d->mAdditionalText = toolStripExp.cap ( 1 );
            d->mStripTitle = toolStripExp.cap ( 2 );
        }
    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void XkcdProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "xkcdprovider.moc"
