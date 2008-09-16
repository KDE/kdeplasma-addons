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

#include <KUrl>

#include "dilbertprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( DilbertProvider, "DilbertProvider", "" )

class DilbertProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };

        QImage mImage;
};

DilbertProvider::DilbertProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setFirstStripDate( QDate ( 1994, 01, 01 ) );
    KUrl url( QString( "http://dilbert.com/fast/%1/" )
                .arg( requestedDate().toString( "yyyy-MM-dd" ) ) );

    MetaInfos infos;
    infos.insert( "User-Agent", "Mozilla/5.0 (compatible; Konqueror/3.5; Linux) KHTML/3.5.6 (like Gecko)" );
    infos.insert( "Accept", "text/html, image/jpeg, image/png, text/*, image/*, */*" );
    infos.insert( "Accept-Encoding", "deflate" );
    infos.insert( "Accept-Charset", "iso-8859-15, utf-8;q=0.5, *;q=0.5" );
    infos.insert( "Accept-Language", "en" );
    infos.insert( "Host", "dilbert.com" );
    infos.insert( "Connection", "Keep-Alive" );

    requestPage( url, Private::PageRequest, infos );
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

void DilbertProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString pattern( "<img src=\"(/dyn/str_strip/[0-9/]+/[0-9]+\\.strip\\.print\\.gif)\"" );
        QRegExp exp( pattern );

        const QString data = QString::fromUtf8( rawData );

        const int pos = exp.indexIn( data );

        KUrl url;

        if ( pos > -1 ) {
            const QString sub = exp.cap( 1 );
            url = KUrl( QString( "http://dilbert.com/%1" ).arg( sub ) );
        } else {
            url = KUrl( QString( "http://dilbert.com/img/v1/404.gif" ) );
        }

        requestPage( url, Private::ImageRequest );
    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void DilbertProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "dilbertprovider.moc"
