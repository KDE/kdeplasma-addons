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

#include "dilbertprovider.h"

class DilbertProvider::Private
{
  public:
    Private( DilbertProvider *parent, const QDate &date )
      : mParent( parent ), mDate( date )
    {
      mHttp = new QHttp( "dilbert.com", 80, mParent );
      connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
    }

    void pageRequestFinished( bool );
    void imageRequestFinished( bool );
    void parsePage();

    DilbertProvider *mParent;
    QByteArray mPage;
    QDate mDate;
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

  const QString pattern( "<IMG SRC=\"/comics/dilbert/archive/images/dilbert" );
  const QRegExp exp( pattern );

  const QString data = QString::fromUtf8( mHttp->readAll() );

  int pos = exp.indexIn( data ) + pattern.length();

  const QString sub = data.mid( pos, data.indexOf( '.', pos ) - pos );

  QUrl url( QString( "http://dilbert.com/comics/dilbert/archive/images/dilbert%1.gif" ).arg( sub ) );

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

  QByteArray data = mImageHttp->readAll();
  mImage = QImage::fromData( data );
  emit mParent->finished( mParent );
}

DilbertProvider::DilbertProvider( const QDate &date, QObject *parent )
    : ComicProvider( parent ), d( new Private( this, date ) )
{
    QUrl url( QString( "http://dilbert.com/comics/dilbert/archive/dilbert-%1.html" ).arg( date.toString( "yyyyMMdd" ) ) );

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

QImage DilbertProvider::image() const
{
    return d->mImage;
}

QString DilbertProvider::identifier() const
{
    return QString( "dilbert:%1" ).arg( d->mDate.toString( Qt::ISODate ) );
}

#include "dilbertprovider.moc"
