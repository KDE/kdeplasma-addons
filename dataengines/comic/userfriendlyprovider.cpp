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

#include "userfriendlyprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( UserFriendlyProvider, "UserFriendlyProvider", "" )

class UserFriendlyProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };
        
        QImage mImage;
};


UserFriendlyProvider::UserFriendlyProvider( QObject *parent, const QVariantList &args )
: ComicProvider( parent, args ), d( new Private )
{
    KUrl url( QString( "http://ars.userfriendly.org/cartoons/?id=" ) + requestedDate().toString( "yyyyMMdd" ) );
    
    MetaInfos infos;
    infos.insert( "User-Agent", "Mozilla/5.0 (compatible; Konqueror/3.5; Linux) KHTML/3.5.6 (like Gecko)" );
    infos.insert( "Accept", "text/html, image/jpeg, image/png, text/*, image/*, */*" );
    infos.insert( "Accept-Encoding", "deflate" );
    infos.insert( "Accept-Charset", "iso-8859-15, utf-8;q=0.5, *;q=0.5" );
    infos.insert( "Accept-Language", "en" );
    infos.insert( "Host", "ars.userfriendly.org" );
    infos.insert( "Referer", QString( "http://ars.userfriendly.org/cartoons/?id=%1" )
    .arg( requestedDate().addDays( -1 ).toString( "yyyyMMdd" ) ) );
    infos.insert( "Connection", "Keep-Alive" );
    
    requestPage( url, Private::PageRequest, infos );
}

UserFriendlyProvider::~UserFriendlyProvider()
{
    delete d;
}

ComicProvider::IdentifierType UserFriendlyProvider::identifierType() const
{
    return DateIdentifier;
}

QImage UserFriendlyProvider::image() const
{
    return d->mImage;
}

QString UserFriendlyProvider::identifier() const
{
    return QString( "userfriendly:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl UserFriendlyProvider::websiteUrl() const
{
    return KUrl( QString( "http://ars.userfriendly.org/cartoons/?id=%1" )
    .arg( requestedDate().toString( "yyyyMMdd" ) ) );
}

void UserFriendlyProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString pattern( "<img border=\"0\" src=\"http://www.userfriendly.org/cartoons/archives/" );
        const QRegExp exp( pattern );
        
        const QString data = QString::fromUtf8( rawData );
        
        const int pos = exp.indexIn( data ) + pattern.length();
        const QString sub = data.mid( pos, data.indexOf( ' ', pos ) - pos - 1 );
        
        KUrl url( QString( "http://ars.userfriendly.org/cartoons/archives/%1" ).arg( sub ) );
        
        requestPage( url, Private::ImageRequest );
    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void UserFriendlyProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "userfriendlyprovider.moc"
