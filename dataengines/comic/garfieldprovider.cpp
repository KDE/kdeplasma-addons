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
#include <QtNetwork/QHttpRequestHeader>

#include <KUrl>

#include "garfieldprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( GarfieldProvider, "GarfieldProvider", "" )

class GarfieldProvider::Private
{
    public:
        Private( GarfieldProvider *parent )
          : mParent( parent )
        {
            mHttp = new QHttp( "images.ucomics.com", 80, mParent );
            connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );
        }

        void imageRequestFinished( bool );

        GarfieldProvider *mParent;
        QImage mImage;

        QHttp *mHttp;
};

void GarfieldProvider::Private::imageRequestFinished( bool error )
{
    if ( error ) {
        emit mParent->error( mParent );
        return;
    }

    mImage = QImage::fromData( mHttp->readAll() );
    emit mParent->finished( mParent );
}

GarfieldProvider::GarfieldProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    KUrl url( QString( "http://images.ucomics.com/comics/ga/%1/ga%2.gif" ).arg( requestedDate().toString( "yyyy" ) )
                                                                          .arg( requestedDate().toString( "yyMMdd" ) ) );

    d->mHttp->setHost( url.host() );
    d->mHttp->get( url.path() );
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

#include "garfieldprovider.moc"
