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
#include <QtNetwork/QHttp>

#include <KUrl>
#include <KDebug>

#include "xkcdprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( XkcdProvider, "XkcdProvider", "" )

class XkcdProvider::Private
{
    public:
        Private( XkcdProvider *parent )
          : mParent( parent ), mHasNextComic( false )
        {
            mHttp = new QHttp( "xkcd.com", 80, mParent );
            connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
        }

        void pageRequestFinished( bool );
        void imageRequestFinished( bool );
        void parsePage();

        XkcdProvider *mParent;
        QImage mImage;
        bool mHasNextComic;
        int mRequestedId;

        QHttp *mHttp;
        QHttp *mImageHttp;
};

void XkcdProvider::Private::pageRequestFinished( bool err )
{
    if ( err ) {
        emit mParent->error( mParent );
        return;
    }

    const QString pattern( "<img src=\"http://imgs.xkcd.com/comics/" );
    const QRegExp exp( pattern );

    const QString data = QString::fromUtf8( mHttp->readAll() );

    const int pos = exp.indexIn( data ) + pattern.length();
    const QString sub = data.mid( pos, data.indexOf( '.', pos ) - pos + 4 );

    KUrl url( QString( "http://imgs.xkcd.com/comics/%1" ).arg( sub ) );

    mImageHttp = new QHttp( "imgs.xkcd.com", 80, mParent );
    mImageHttp->setHost( url.host() );
    mImageHttp->get( url.path() );

    mParent->connect( mImageHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );

    // search the id of this comic if it was not specified
    if ( mRequestedId < 1 ) {
        const QString idPattern( "http://xkcd.com/(\\d+)/" );
        QRegExp idExp( idPattern );

        if ( idExp.indexIn( data ) > -1 ) {
            mRequestedId = idExp.cap( 1 ).toInt();
        }
    }


    // now search if there is a next comic
    const QString nextPattern( "href=\"#\"" );
    const QRegExp nextExp( nextPattern );

    mHasNextComic = (nextExp.indexIn( data ) == -1);
}

void XkcdProvider::Private::imageRequestFinished( bool error )
{
    if ( error ) {
        emit mParent->error( mParent );
        return;
    }

    mImage = QImage::fromData( mImageHttp->readAll() );
    emit mParent->finished( mParent );
}

XkcdProvider::XkcdProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    d->mRequestedId = requestedNumber();

    KUrl baseUrl( QString( "http://xkcd.com/" ) );

    if ( d->mRequestedId > 0 ) {
        baseUrl.setPath( QString::number( d->mRequestedId ) + '/' );
    }

    d->mHttp->setHost( baseUrl.host() );
    d->mHttp->get( baseUrl.path() );
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

#include "xkcdprovider.moc"
