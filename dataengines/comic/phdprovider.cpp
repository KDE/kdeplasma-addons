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
#include <QtNetwork/QHttp>

#include <KUrl>

#include "phdprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( PhdProvider, "PhdProvider", "" )

class PhdProvider::Private
{
    public:
        Private( PhdProvider *parent )
          : mParent( parent ), mHasNextComic( false )
        {
            mHttp = new QHttp( "www.phdcomics.com", 80, mParent );
            connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
        }

        void pageRequestFinished( bool );
        void imageRequestFinished( bool );
        void parsePage();

        PhdProvider *mParent;
        QImage mImage;
        bool mHasNextComic;
        int mRequestedId;

        QHttp *mHttp;
        QHttp *mImageHttp;
};

void PhdProvider::Private::pageRequestFinished( bool err )
{
    if ( err ) {
        emit mParent->error( mParent );
        return;
    }

    const QString pattern( "\\<img src=http://www\\.phdcomics\\.com/comics/archive/" );
    const QRegExp exp( pattern );

    const QString data = QString::fromUtf8( mHttp->readAll() );

    const int pos = exp.indexIn( data ) + pattern.length();
    const QString sub = data.mid( pos, data.indexOf( ' ', pos ) - pos );

    KUrl url( QString( "http://www.phdcomics.com/comics/archive/phd%1" ).arg( sub ) );

    mImageHttp = new QHttp( "www.phdcomics.com", 80, mParent );
    mImageHttp->setHost( url.host() );
    mImageHttp->get( url.path() );

    mParent->connect( mImageHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );

    // search the id of this comic if it was not specified
    if ( mRequestedId < 1 ) {
        const QString idPattern( "tellafriend\\.php\\?comicid=(\\d+)" );
        QRegExp idExp( idPattern );

        if ( idExp.indexIn( data ) > -1 ) {
            mRequestedId = idExp.cap( 1 ).toInt();
        }
    }

    // now search if there is a next comic
    const QString nextPattern( "images/next_button.gif" );
    const QRegExp nextExp( nextPattern );

    mHasNextComic = (nextExp.indexIn( data ) != -1);
}

void PhdProvider::Private::imageRequestFinished( bool error )
{
    if ( error ) {
        emit mParent->error( mParent );
        return;
    }

    mImage = QImage::fromData( mImageHttp->readAll() );
    emit mParent->finished( mParent );
}

PhdProvider::PhdProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    d->mRequestedId = requestedNumber();

    KUrl baseUrl( QString( "http://www.phdcomics.com/" ) );

    if ( d->mRequestedId > 0 ) {
        baseUrl.setPath( QString("/comics/archive.php?comicid=%1").arg( d->mRequestedId ) );
    } else {
        baseUrl.setPath("/comics/archive.php");
    }

    d->mHttp->setHost( baseUrl.host() );
    d->mHttp->get( baseUrl.path() );
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

#include "phdprovider.moc"
