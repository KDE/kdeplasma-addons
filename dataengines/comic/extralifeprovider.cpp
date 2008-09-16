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

#include "extralifeprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( ExtraLifeProvider, "ExtraLifeProvider", "" )

class ExtraLifeProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };

        Private( ExtraLifeProvider *parent )
            : mUsedDate( parent->requestedDate() )
        {
        }

        QImage mImage;
        QDate mUsedDate;
        bool mFindNewDate;
        QDate mPreviousDate;
        QDate mNextDate;
};


ExtraLifeProvider::ExtraLifeProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    setFirstStripDate( QDate( 2001, 06, 17 ) );
    d->mFindNewDate = (d->mUsedDate == QDate::currentDate());

    setWebsiteHttp();
}

void ExtraLifeProvider::setWebsiteHttp()
{
    KUrl url( QString( "http://myextralife.com/" ) );

    if ( d->mFindNewDate ) {
        url.setPath( QString( "/archive.php" ) );
    } else {
        url.setPath( "/archive.php" );
        url.addQueryItem( "date", d->mUsedDate.toString( "yyyy-MM-dd" ) );
    }

    requestPage( url, Private::PageRequest );
}

ExtraLifeProvider::~ExtraLifeProvider()
{
    delete d;
}

ComicProvider::IdentifierType ExtraLifeProvider::identifierType() const
{
    return DateIdentifier;
}

QImage ExtraLifeProvider::image() const
{
    return d->mImage;
}

QString ExtraLifeProvider::identifier() const
{
    return QString( "extralife:%1" ).arg( d->mUsedDate.toString( Qt::ISODate ) );
}

KUrl ExtraLifeProvider::websiteUrl() const
{
    return QString( "http://www.myextralife.com/archive.php?date=%1" )
    .arg( d->mUsedDate.toString( Qt::ISODate ) );
}

QString ExtraLifeProvider::nextIdentifier() const
{
    return d->mNextDate.toString( Qt::ISODate );
}

QString ExtraLifeProvider::previousIdentifier() const
{
    return d->mPreviousDate.toString( Qt::ISODate );
}

void ExtraLifeProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString data = QString::fromUtf8( rawData );

        const QString year( "((19|20)\\d\\d)" );
        const QString month( "(0[1-9]|1[012])" );
        const QString day( "(0[1-9]|[12][0-9]|3[01])" );
        QRegExp exp( "<img src=\"strips/(.+)\">" );
        exp.setMinimal( true );
        const int pos = exp.indexIn( data );

        KUrl url;

        if ( pos > -1 ) {
            if ( d->mFindNewDate ) {
                QRegExp expNoDate( "strips/(" + month + "-" + day + "-" + year + ")\\.jpg" );
                const int posNoDate = expNoDate.indexIn( data );
                if ( posNoDate > -1 ) {
                    d->mUsedDate =  QDate::fromString( expNoDate.cap( 1 ), "MM-dd-yyyy" );
                    d->mFindNewDate = false;
                    setWebsiteHttp();
                }
                return;
            }

            url = KUrl( QString( "http://myextralife.com/strips/%1" ).arg( exp.cap( 1 ) ) );
        }

        const QString patternDate( "(" + year + "-" + month + "-" + day + ")" );

        QRegExp expPrev( "href=\"archive.php\\?date=" + patternDate + "\"> <IMG src=images/back_strip\\.png" );
        const int posPrev = expPrev.indexIn( data );

        if ( posPrev > -1 ) {
            d->mPreviousDate = QDate::fromString( expPrev.cap( 1 ), "yyyy-MM-dd" );
        }

        QRegExp expNext( "href=\"archive.php\\?date=" + patternDate + "\"> <IMG src=images/forward_strip\\.png" );
        const int posNext = expNext.indexIn( data );

        if ( posNext > -1 ) {
            d->mNextDate = QDate::fromString( expNext.cap( 1 ), "yyyy-MM-dd" );
        }

        requestPage( url, Private::ImageRequest );

    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void ExtraLifeProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "extralifeprovider.moc"
