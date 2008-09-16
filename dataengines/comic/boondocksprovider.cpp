/*
*   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
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

#include "boondocksprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( BoondocksProvider, "BoondocksProvider", "" )

class BoondocksProvider::Private
{
    public:
        QImage mImage;
};

BoondocksProvider::BoondocksProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setFirstStripDate( QDate( 1999, 04, 19 ) );
    KUrl url( QString( "http://picayune.uclick.com/comics/bo/%1/bo%2.gif" ).arg( requestedDate().toString( "yyyy" ) )
                                                                          .arg( requestedDate().toString( "yyMMdd" ) ) );

    requestPage( url, 0 );
}

BoondocksProvider::~BoondocksProvider()
{
    delete d;
}

ComicProvider::IdentifierType BoondocksProvider::identifierType() const
{
    return DateIdentifier;
}

QImage BoondocksProvider::image() const
{
    return d->mImage;
}

QString BoondocksProvider::identifier() const
{
    return QString( "boondocks:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl BoondocksProvider::websiteUrl() const
{
    return QString( "http://www.gocomics.com/boondocks/%1/" ).arg( requestedDate().toString( "yyyy/MM/dd" ) );
}

void BoondocksProvider::pageRetrieved( int, const QByteArray &data )
{
    d->mImage = QImage::fromData( data );

    emit finished( this );
}

void BoondocksProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "boondocksprovider.moc"
