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

// Qt
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QRegExp>
#include <QtGui/QImage>

// KDE
#include <KUrl>
#include <syndication/item.h>

#include "osnewsprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( OsNewsProvider, "OsNewsProvider", "" )

class OsNewsProvider::Private
{
    public:
        Private( OsNewsProvider *parent )
          : mParent( parent )
        {
        }

        void processRss( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode );

        OsNewsProvider *mParent;
        QDate mCurrentDate;
        QDate mPreviousDate;
        QDate mNextDate;
        QString mTitle;
        QImage mImage;

        Syndication::Loader *mLoader;

        KUrl mPageUrl;
};

void OsNewsProvider::Private::processRss( Syndication::Loader*, Syndication::FeedPtr feed, Syndication::ErrorCode error )
{
    if ( error != Syndication::Success ) {
        emit mParent->error( mParent );
    } else {
        QVariantList items;

        QDateTime itemDate;
        QDate tempDate;
        QDate tempNextDate;
        QString description;

        foreach ( const Syndication::ItemPtr& item, feed->items() ) {
            itemDate.setTime_t( item->datePublished() );
            if ( !tempDate.isNull() ) {
                tempNextDate = tempDate;
            }
            tempDate = QDate( itemDate.date() );
            if ( tempDate <= mParent->requestedDate() ) {
                if ( mPageUrl.isEmpty() ) {
                    mPageUrl = item->link();
                    mTitle = item->title();
                    description = item->description();
                    mCurrentDate = tempDate;
                    mNextDate = tempNextDate;
                } else {
                    mPreviousDate = tempDate;;
                    break;
                }
            }
        }

        const int start = description.indexOf( "http://" );
        const int end = description.indexOf ( "\"", start );
        const QString imgUrl = description.mid( start, end - start );

        if ( !mPageUrl.isEmpty() ) {
            KUrl url( imgUrl );
            mParent->requestPage( "www.osnews.com", 80, url.path(), 0 );
        } else {
            // this should never happen
            emit mParent->error( mParent );
        }
    }
}

OsNewsProvider::OsNewsProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    KUrl url( "http://osnews.com/files/comics.xml" );

    d->mLoader = Syndication::Loader::create();
    connect( d->mLoader, SIGNAL( loadingComplete( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ),
             this, SLOT( processRss( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ) );

    d->mLoader->loadFrom( url );
}

OsNewsProvider::~OsNewsProvider()
{
    delete d;
}

ComicProvider::IdentifierType OsNewsProvider::identifierType() const
{
    return DateIdentifier;
}

KUrl OsNewsProvider::websiteUrl() const
{
    return d->mPageUrl;
}

QString OsNewsProvider::nextIdentifier() const
{
    return d->mNextDate.toString( Qt::ISODate );
}

QString OsNewsProvider::previousIdentifier() const
{
    return d->mPreviousDate.toString( Qt::ISODate );
}

QImage OsNewsProvider::image() const
{
    return d->mImage;
}

QString OsNewsProvider::identifier() const
{
    return QString( "osnews:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

void OsNewsProvider::pageRetrieved( int, const QByteArray &data )
{
    d->mImage = QImage::fromData( data );

    emit finished( this );
}

void OsNewsProvider::pageError( int, const QString& )
{
    emit error( this );
}


#include "osnewsprovider.moc"
