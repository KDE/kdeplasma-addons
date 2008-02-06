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

#include "xkcdprovider.h"

class XkcdProvider::Private
{
  public:
    Private( XkcdProvider *parent, const QDate &date )
      : mParent( parent ), mDate( date )
    {
      mHttp = new QHttp( "xkcd.com", 80, mParent );
      connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
    }

    void pageRequestFinished( bool );
    void imageRequestFinished( bool );
    void parsePage();

    XkcdProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;

    QHttp *mHttp;
    QHttp *mImageHttp;
};

static QString dateToId( const QDate &date )
{
    static QDate initialDate = QDate( 2006, 12, 6 ); // when everything started...

    if ( date < initialDate )
        return QString();

    int days = initialDate.daysTo( date );

    return QString::number( days );
}

void XkcdProvider::Private::pageRequestFinished( bool err )
{
  if ( err ) {
    emit mParent->error( mParent );
    return;
  }

  const QString pattern( "<img src=\"http://imgs.xkcd.com/comics/" );
  const QRegExp exp( pattern );

  const QString data = QString::fromUtf8( mHttp->readAll() );

  int pos = exp.indexIn( data ) + pattern.length();

  const QString sub = data.mid( pos, data.indexOf( '.', pos ) - pos + 4 );

  KUrl url( QString( "http://imgs.xkcd.com/comics/%1" ).arg( sub ) );

  mImageHttp = new QHttp( "imgs.xkcd.com", 80, mParent );
  mImageHttp->setHost( url.host() );
  mImageHttp->get( url.path() );

  mParent->connect( mImageHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );
}

void XkcdProvider::Private::imageRequestFinished( bool error )
{
  if ( error ) {
    emit mParent->error( mParent );
    return;
  }

  QByteArray data = mImageHttp->readAll();
  mImage = QImage::fromData( data );
  emit mParent->finished( mParent );
}

XkcdProvider::XkcdProvider( const QDate &date, QObject *parent )
    : ComicProvider( parent ), d( new Private( this, date ) )
{
    KUrl url( QString( "http://xkcd.com/%1/" ).arg( dateToId( date ) ) );

    d->mHttp->setHost( url.host() );
    d->mHttp->get( url.path() );
}

XkcdProvider::~XkcdProvider()
{
    delete d;
}

QImage XkcdProvider::image() const
{
    return d->mImage;
}

QString XkcdProvider::identifier() const
{
    return QString( "xkcd:%1" ).arg( d->mDate.toString( Qt::ISODate ) );
}

KUrl XkcdProvider::websiteUrl() const
{
    return QString( "http://xkcd.com/%1/" ).arg( dateToId( d->mDate ) );
}

#include "xkcdprovider.moc"
