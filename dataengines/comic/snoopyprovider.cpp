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
#include <QtCore/QRegExp>
#include <QtGui/QImage>
#include <QtNetwork/QHttp>

#include <KUrl>

#include "snoopyprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( SnoopyProvider, "SnoopyProvider", "" )

class SnoopyProvider::Private
{
    public:
        Private( SnoopyProvider *parent )
          : mParent( parent )
        {
            mHttp = new QHttp( "snoopy.com", 80, mParent );
            connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
        }

        void pageRequestFinished( bool );
        void imageRequestFinished( bool );
        void parsePage();

        SnoopyProvider *mParent;
        QImage mImage;

        QHttp *mHttp;
        QHttp *mImageHttp;
};

void SnoopyProvider::Private::pageRequestFinished( bool err )
{
    if ( err ) {
        emit mParent->error( mParent );
        return;
    }

    const QString pattern( "<IMG SRC=\"/comics/peanuts/archive/images/peanuts" );
    const QRegExp exp( pattern );

    const QString data = QString::fromUtf8( mHttp->readAll() );

    const int pos = exp.indexIn( data ) + pattern.length();
    const QString sub = data.mid( pos, data.indexOf( '.', pos ) - pos );

    KUrl url( QString( "http://snoopy.com/comics/peanuts/archive/images/peanuts%1.gif" ).arg( sub ) );

    mImageHttp = new QHttp( "snoopy.com", 80, mParent );
    mImageHttp->setHost( url.host() );
    mImageHttp->get( url.path() );

    mParent->connect( mImageHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );
}

void SnoopyProvider::Private::imageRequestFinished( bool error )
{
    if ( error ) {
        emit mParent->error( mParent );
        return;
    }

    mImage = QImage::fromData( mImageHttp->readAll() );
    emit mParent->finished( mParent );
}

SnoopyProvider::SnoopyProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    KUrl url( QString( "http://snoopy.com/comics/peanuts/archive/peanuts-%1.html" )
                .arg( requestedDate().toString( "yyyyMMdd" ) ) );

    QHttpRequestHeader header( "GET", url.path() );
    header.setValue( "User-Agent", "Mozilla/5.0 (compatible; Konqueror/3.5; Linux) KHTML/3.5.6 (like Gecko)" );
    header.setValue( "Accept", "text/html, image/jpeg, image/png, text/*, image/*, */*" );
    header.setValue( "Accept-Encoding", "deflate" );
    header.setValue( "Accept-Charset", "iso-8859-15, utf-8;q=0.5, *;q=0.5" );
    header.setValue( "Accept-Language", "en" );
    header.setValue( "Host", "snoopy.com" );
    header.setValue( "Connection", "Keep-Alive" );

    d->mHttp->setHost( url.host() );
    d->mHttp->request( header );
}

SnoopyProvider::~SnoopyProvider()
{
    delete d;
}

ComicProvider::IdentifierType SnoopyProvider::identifierType() const
{
    return DateIdentifier;
}

QImage SnoopyProvider::image() const
{
    return d->mImage;
}

QString SnoopyProvider::identifier() const
{
    return QString( "snoopy:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl SnoopyProvider::websiteUrl() const
{
    return QString( "http://snoopy.com/comics/peanuts/archive/peanuts-%1.html" )
             .arg( requestedDate().toString( "yyyyMMdd" ) );
}

#include "snoopyprovider.moc"
