/*
 *   Copyright (C) 2007 Marco Martin  <notmart@gmail.com>
 *   derived from DilbertProvider by
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

//Qt
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QRegExp>
#include <QtGui/QImage>
#include <QHash>
#include <QtNetwork/QHttp>
#include <QtNetwork/QHttpRequestHeader>

//Kde
#include <KUrl>
#include <syndication/item.h>
#include <KDebug>

//own
#include "osnewsprovider.h"

class OsNewsProvider::Private
{
  public:
    Private( OsNewsProvider *parent, const QDate &date )
      : mParent( parent ), mAskedDate( date )
    {
    }

    void pageRequestFinished( bool );
    void imageRequestFinished( bool );
    void processRss(Syndication::Loader* loader,
                    Syndication::FeedPtr feed,
                    Syndication::ErrorCode error);

    OsNewsProvider *mParent;
    QByteArray mPage;
    QDate mAskedDate;
    QDate mCurrentDate;
    QDate mPreviousDate;
    QDate mNextDate;
    QString mTitle;
    QImage mImage;

    Syndication::Loader *loader;

    QHttp *mPageHttp;
    QHttp *mImageHttp;

    KUrl mPageUrl;
};

void OsNewsProvider::Private::pageRequestFinished( bool err )
{
  if ( err ) {
    emit mParent->error( mParent );
    return;
  }

  const QString pattern( "<img src=\"http://www.osnews.com/images/comics/" );
  const QRegExp exp( pattern );

  const QString data = QString::fromUtf8( mPageHttp->readAll() );

  int pos = exp.indexIn( data ) + pattern.length();
  const QString sub = data.mid( pos, data.indexOf( '"', pos ) - pos );

  KUrl url( QString( "http://www.osnews.com/images/comics/%1" ).arg( sub ) );

  mImageHttp = new QHttp( "osnews.com", 80, mParent );
  mImageHttp->setHost( url.host() );
  mImageHttp->get( url.path() );

  mParent->connect( mImageHttp, SIGNAL( done( bool ) ), mParent, SLOT( imageRequestFinished( bool ) ) );
}

void OsNewsProvider::Private::imageRequestFinished( bool error )
{
  if ( error ) {
    emit mParent->error( mParent );
    return;
  }

  QByteArray data = mImageHttp->readAll();
  mImage = QImage::fromData( data );
  emit mParent->finished( mParent );
}

void OsNewsProvider::Private::processRss(Syndication::Loader* loader,
                           Syndication::FeedPtr feed,
                           Syndication::ErrorCode error)
{
    if (error != Syndication::Success) {
        emit mParent->error( mParent );
    } else {
        QVariantList items;

        QDateTime itemDate;
        QDate tempDate;
        QDate tempNextDate;

        foreach (const Syndication::ItemPtr& item, feed->items()) {
            itemDate.setTime_t(item->datePublished());
            if( !tempDate.isNull() ) {
                tempNextDate = tempDate;
            }
            tempDate = QDate(itemDate.date());
            if (tempDate <= mAskedDate) {
                if (mPageUrl.isEmpty()) {
                    mPageUrl = item->link();
                    mTitle = item->title();
                    mCurrentDate = tempDate;
                    mNextDate = tempNextDate;
                } else {
                    mPreviousDate = tempDate;;
                    break;
                }
            }
        }

        kDebug() << "Comic webpage found: " << mPageUrl;

        if (!mPageUrl.isEmpty()) {
            mPageHttp = new QHttp( "osnews.com", 80, mParent );
            mPageHttp->setHost( mPageUrl.host() );
            mPageHttp->get( mPageUrl.path() );
            connect( mPageHttp, SIGNAL( done( bool ) ), mParent, SLOT( pageRequestFinished( bool ) ) );
        } else {
            //this should never happen
            emit mParent->error( mParent );
        }
    }
}

OsNewsProvider::OsNewsProvider( const QDate &date, QObject *parent )
    : ComicProvider( parent ), d( new Private( this, date ) )
{
    KUrl url( "http://osnews.com/feed/topic/79" );

    d->loader = Syndication::Loader::create();
    connect(d->loader, SIGNAL(loadingComplete(Syndication::Loader*,
                                           Syndication::FeedPtr,
                                           Syndication::ErrorCode)),
            this, SLOT(processRss(Syndication::Loader*,
                                  Syndication::FeedPtr,
                                  Syndication::ErrorCode)));
    d->loader->loadFrom(url);
}

OsNewsProvider::~OsNewsProvider()
{
    delete d;
}

KUrl OsNewsProvider::websiteUrl() const
{
   return d->mPageUrl;
}

QString OsNewsProvider::nextIdentifierSuffix() const
{
   return d->mNextDate.toString(Qt::ISODate);
}

QString OsNewsProvider::previousIdentifierSuffix() const
{
   return d->mPreviousDate.toString(Qt::ISODate);
}

QImage OsNewsProvider::image() const
{
    return d->mImage;
}

QString OsNewsProvider::identifier() const
{
    return QString( "osnews:%1" ).arg( d->mAskedDate.toString( Qt::ISODate ) );
}

#include "osnewsprovider.moc"
