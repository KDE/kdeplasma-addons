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

#include "garfieldprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( GarfieldProvider, "GarfieldProvider", "" )

class GarfieldProvider::Private
{
    public:
        QImage mImage;
};

GarfieldProvider::GarfieldProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setComicAuthor( "Jim Davis" );
    setFirstStripDate( QDate( 1978, 06, 19 ) );
    KUrl url( QString( "http://picayune.uclick.com/comics/ga/%1/ga%2.gif" ).arg( requestedDate().toString( "yyyy" ) )
                                                                          .arg( requestedDate().toString( "yyMMdd" ) ) );

    requestPage( url, 0 );
}

GarfieldProvider::~GarfieldProvider()
{
    delete d;
}

ComicProvider::IdentifierType GarfieldProvider::identifierType() const
{
    return DateIdentifier;
}

QImage GarfieldProvider::image() const
{
    return d->mImage;
}

QString GarfieldProvider::identifier() const
{
    return QString( "garfield:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl GarfieldProvider::websiteUrl() const
{
    return QString( "http://www.gocomics.com/garfield/%1/" ).arg( requestedDate().toString( "yyyy/MM/dd" ) );
}

void GarfieldProvider::pageRetrieved( int, const QByteArray &data )
{
    d->mImage = QImage::fromData( data );

    emit finished( this );
}

void GarfieldProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "garfieldprovider.moc"
