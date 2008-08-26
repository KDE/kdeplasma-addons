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

#include "snoopyprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( SnoopyProvider, "SnoopyProvider", "" )

class SnoopyProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };

        QImage mImage;
};


SnoopyProvider::SnoopyProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    KUrl url( QString( "http://snoopy.com/comics/peanuts/archive/peanuts-%1.html" )
                .arg( requestedDate().toString( "yyyyMMdd" ) ) );

    requestPage( "snoopy.com", 80, url.path(), Private::PageRequest );
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

void SnoopyProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString pattern( "<IMG SRC=\"/comics/peanuts/archive/images/peanuts" );
        const QRegExp exp( pattern );

        const QString data = QString::fromUtf8( rawData );

        const int pos = exp.indexIn( data ) + pattern.length();
        const QString sub = data.mid( pos, data.indexOf( '"', pos ) - pos );

        KUrl url( QString( "http://snoopy.com/comics/peanuts/archive/images/peanuts%1" ).arg( sub ) );

        requestPage( "snoopy.com", 80, url.path(), Private::ImageRequest );
    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void SnoopyProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "snoopyprovider.moc"
