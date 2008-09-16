/*
 *   Copyright (C) 2008 Heiko Jansen <heiko_jansen@web.de>
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

#include "unshelvedprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( UnshelvedProvider, "UnshelvedProvider", "" )

class UnshelvedProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };

        QImage mImage;
};


UnshelvedProvider::UnshelvedProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setFirstStripDate( QDate( 2002, 02, 16 ) );
    // as unshelved.com seem to publish their comic quite late the day
    // we fake it here by always showing the comic of the day before...
    const QDate date = requestedDate().addDays( -1 );

    KUrl url( QString( "http://www.unshelved.com/strips/%1.gif" )
                .arg( date.toString( "yyyyMMdd" ) ) );

    requestPage( url, 0 );
}

UnshelvedProvider::~UnshelvedProvider()
{
    delete d;
}

ComicProvider::IdentifierType UnshelvedProvider::identifierType() const
{
    return DateIdentifier;
}

QImage UnshelvedProvider::image() const
{
    return d->mImage;
}

QString UnshelvedProvider::identifier() const
{
    return QString( "unshelved:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl UnshelvedProvider::websiteUrl() const
{
    return QString( "http://unshelved.com/archive.aspx?strip=%1" )
                 .arg( requestedDate().toString( "yyyyMMdd" ) );
}

void UnshelvedProvider::pageRetrieved( int, const QByteArray &data )
{
    d->mImage = QImage::fromData( data );
    emit finished( this );
}

void UnshelvedProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "unshelvedprovider.moc"
