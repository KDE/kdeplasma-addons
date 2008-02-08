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

#include "userfriendlyprovider.h"

class UserFriendlyProvider::Private
{
  public:
    Private( UserFriendlyProvider *parent, const QDate &date )
      : mParent( parent ), mDate( date )
    {
      mHttp = new QHttp( "ars.userfriendly.org", 80, mParent );
      connect( mHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
    }

    void pageRequestFinished( bool );
    void imageRequestFinished( bool );
    void parsePage();

    UserFriendlyProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;

    QHttp *mHttp;
    QHttp *mImageHttp;
};

void UserFriendlyProvider::Private::pageRequestFinished( bool err )
{
  if ( err ) {
    emit mParent->error( mParent );
    return;
  }

  const QString pattern( "<img border=\"0\" src=\"http://www.userfriendly.org/cartoons/archives/" );
  const QRegExp exp( pattern );

  const QString data = QString::fromUtf8( mHttp->readAll() );

  int pos = exp.indexIn( data ) + pattern.length();

  const QString sub = data.mid( pos, data.indexOf( ' ', pos ) - pos - 1 );

  KUrl url( QString( "http://www.userfriendly.org/cartoons/archives/%1" ).arg( sub ) );

  mImageHttp = new QHttp( "ars.userfriendly.org", 80, mParent );
  mImageHttp->setHost( url.host() );
  mImageHttp->get( url.path() );

  mParent->connect( mImageHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );
}

void UserFriendlyProvider::Private::imageRequestFinished( bool error )
{
  if ( error ) {
    emit mParent->error( mParent );
    return;
  }

  mImage = QImage::fromData( mImageHttp->readAll() );
  emit mParent->finished( mParent );
}

UserFriendlyProvider::UserFriendlyProvider( const QDate &date, QObject *parent )
    : ComicProvider( parent ), d( new Private( this, date ) )
{
    QString path( QString( "/cartoons/?id=" ) + date.toString( "yyyyMMdd" ) );

    QHttpRequestHeader header( "GET", path );
    header.setValue( "User-Agent", "Mozilla/5.0 (compatible; Konqueror/3.5; Linux) KHTML/3.5.6 (like Gecko)" );
    header.setValue( "Accept", "text/html, image/jpeg, image/png, text/*, image/*, */*" );
    header.setValue( "Accept-Encoding", "deflate" );
    header.setValue( "Accept-Charset", "iso-8859-15, utf-8;q=0.5, *;q=0.5" );
    header.setValue( "Accept-Language", "en" );
    header.setValue( "Host", "ars.userfriendly.org" );
    header.setValue( "Referer", QString( "http://ars.userfriendly.org/cartoons/?id=%1" )
                                   .arg( date.addDays( -1 ).toString( "yyyyMMdd" ) ) );
    header.setValue( "Connection", "Keep-Alive" );

    d->mHttp->setHost( "ars.userfriendly.org" );
    d->mHttp->request( header );
}

UserFriendlyProvider::~UserFriendlyProvider()
{
    delete d;
}

QImage UserFriendlyProvider::image() const
{
    return d->mImage;
}

QString UserFriendlyProvider::identifier() const
{
    return QString( "userfriendly:%1" ).arg( d->mDate.toString( Qt::ISODate ) );
}

KUrl UserFriendlyProvider::websiteUrl() const
{
    return KUrl(QString( "http://ars.userfriendly.org/cartoons/?id=" ) + d->mDate.toString( "yyyyMMdd" ));
}

QString UserFriendlyProvider::nextIdentifierSuffix() const
{
   if (d->mDate < QDate::currentDate()) {
       return d->mDate.addDays(+1).toString( Qt::ISODate );
   } else {
       return QString();
   }
}

QString UserFriendlyProvider::previousIdentifierSuffix() const
{
   //comic born 17 Now 1997
   if (d->mDate > QDate(1997, 11, 17)) {
       return d->mDate.addDays(-1).toString( Qt::ISODate );
   } else {
       return QString();
   }
}

#include "userfriendlyprovider.moc"
