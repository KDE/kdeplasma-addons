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
#include <QtCore/QUrl>
#include <QtGui/QImage>
#include <QtNetwork/QHttp>
#include <QtNetwork/QHttpRequestHeader>

#include "garfieldprovider.h"

class GarfieldProvider::Private
{
  public:
    Private( GarfieldProvider *parent, const QDate &date )
      : mParent( parent ), mDate( date )
    {
      mHttp = new QHttp( "images.ucomics.com", 80, mParent );
      connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );
    }

    void imageRequestFinished( bool );

    GarfieldProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;

    QHttp *mHttp;
};

void GarfieldProvider::Private::imageRequestFinished( bool error )
{
    if ( error ) {
        emit mParent->error( mParent );
        return;
    }

    QByteArray data = mHttp->readAll();
    mImage = QImage::fromData( data );
    emit mParent->finished( mParent );
}

GarfieldProvider::GarfieldProvider( const QDate &date, QObject *parent )
    : ComicProvider( parent ), d( new Private( this, date ) )
{
    QUrl url( QString( "http://images.ucomics.com/comics/ga/%1/ga%2.gif" ).arg( date.toString( "yyyy" ) )
                                                                          .arg( date.toString( "yyMMdd" ) ) );

    d->mHttp->setHost( url.host() );
    d->mHttp->get( url.path() );
}

GarfieldProvider::~GarfieldProvider()
{
    delete d;
}

QImage GarfieldProvider::image() const
{
    return d->mImage;
}

QString GarfieldProvider::identifier() const
{
    return QString( "garfield:%1" ).arg( d->mDate.toString( Qt::ISODate ) );
}

#include "garfieldprovider.moc"
