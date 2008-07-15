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

#include "dilbertprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( DilbertProvider, "DilbertProvider", "" )

class DilbertProvider::Private
{
    public:
        Private( DilbertProvider *parent )
          : mParent( parent )
        {
            mHttp = new QHttp( "dilbert.com", 80, mParent );
            connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
        }

        void pageRequestFinished( bool );
        void imageRequestFinished( bool );
        void parsePage();

        DilbertProvider *mParent;
        QImage mImage;

        QHttp *mHttp;
        QHttp *mImageHttp;
};

void DilbertProvider::Private::pageRequestFinished( bool err )
{
    if ( err ) {
        emit mParent->error( mParent );
        return;
    }

    const QString pattern( "<img src=\"(/dyn/str_strip/[0-9/]+/[0-9]+\\.strip\\.print\\.gif)\"" );
    QRegExp exp( pattern );

    const QString data = QString::fromUtf8( mHttp->readAll() );

    const int pos = exp.indexIn( data );

    KUrl url;

    if (pos > -1) {
        QString sub = exp.cap(1);
        url = KUrl( QString( "http://dilbert.com/%1" ).arg( sub ) );
    } else {
        url = KUrl( QString( "http://dilbert.com/img/v1/404.gif" ) );
    }

    mImageHttp = new QHttp( "dilbert.com", 80, mParent );
    mImageHttp->setHost( url.host() );
    mImageHttp->get( url.path() );

    mParent->connect( mImageHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );
}

void DilbertProvider::Private::imageRequestFinished( bool error )
{
    if ( error ) {
        emit mParent->error( mParent );
        return;
    }

    mImage = QImage::fromData( mImageHttp->readAll() );
    emit mParent->finished( mParent );
}

DilbertProvider::DilbertProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    KUrl url( QString( "http://dilbert.com/strips/comic/%1/" )
                .arg( requestedDate().toString( "yyyy-MM-dd" ) ) );

    QHttpRequestHeader header( "GET", url.path() );
    header.setValue( "User-Agent", "Mozilla/5.0 (compatible; Konqueror/3.5; Linux) KHTML/3.5.6 (like Gecko)" );
    header.setValue( "Accept", "text/html, image/jpeg, image/png, text/*, image/*, */*" );
    header.setValue( "Accept-Encoding", "deflate" );
    header.setValue( "Accept-Charset", "iso-8859-15, utf-8;q=0.5, *;q=0.5" );
    header.setValue( "Accept-Language", "en" );
    header.setValue( "Host", "dilbert.com" );
    header.setValue( "Connection", "Keep-Alive" );

    d->mHttp->setHost( url.host() );
    d->mHttp->request( header );
}

DilbertProvider::~DilbertProvider()
{
    delete d;
}

ComicProvider::IdentifierType DilbertProvider::identifierType() const
{
    return DateIdentifier;
}

QImage DilbertProvider::image() const
{
    return d->mImage;
}

QString DilbertProvider::identifier() const
{
    return QString( "dilbert:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl DilbertProvider::websiteUrl() const
{
    return QString( "http://dilbert.com/strips/comic/%1/" )
             .arg( requestedDate().toString( "yyyy-MM-dd" ) );
}

#include "dilbertprovider.moc"
