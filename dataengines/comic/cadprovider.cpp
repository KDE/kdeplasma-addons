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

#include "cadprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( CadProvider, "CadProvider", "" )

class CadProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };

        Private( CadProvider *parent )
            : mUsedDate( parent->requestedDate() )
        {
        }

        QImage mImage;
        QDate mUsedDate;
        bool mFindNewDate;
        QDate mPreviousDate;
        QDate mNextDate;
};


CadProvider::CadProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    d->mFindNewDate = (d->mUsedDate == QDate::currentDate());

    setWebsiteHttp();
}

void CadProvider::setWebsiteHttp()
{
    KUrl url( QString( "http://cad-comic.com/" ) );

    if ( d->mFindNewDate )
        url.setPath( QString( "/comic.php" ) );
    else {
        url.setPath( "/comic.php" );
        url.addQueryItem( "d", d->mUsedDate.toString( "yyyyMMdd" ) );
    }

    requestPage( url, Private::PageRequest );
}

CadProvider::~CadProvider()
{
    delete d;
}

ComicProvider::IdentifierType CadProvider::identifierType() const
{
    return DateIdentifier;
}

QImage CadProvider::image() const
{
    return d->mImage;
}

QString CadProvider::identifier() const
{
    return QString( "cad:%1" ).arg( d->mUsedDate.toString( Qt::ISODate ) );
}

KUrl CadProvider::websiteUrl() const
{
    return QString( "http://cad-comic.com/comic.php?d=%1" ).arg( d->mUsedDate.toString( "yyyyMMdd" ) );
}

QString CadProvider::nextIdentifier() const
{
    return d->mNextDate.toString( Qt::ISODate );
}

QString CadProvider::previousIdentifier() const
{
    return d->mPreviousDate.toString( Qt::ISODate );
}

void CadProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString data = QString::fromUtf8( rawData );
        const QString patternDate( "(((19|20)\\d\\d)(0[1-9]|1[012])(0[1-9]|[12][0-9]|3[01]))" );
        QRegExp exp( "(" + patternDate + "\\.jpg)");
        const int pos = exp.indexIn( data );

        KUrl url;

        if ( pos > -1 ) {
            if ( d->mFindNewDate ) {
                d->mUsedDate =  QDate::fromString( exp.cap( 2 ), "yyyyMMdd" );
                d->mFindNewDate = false;
                setWebsiteHttp();
                return;
            }

            QString sub = exp.cap( 1 );
            url = KUrl( QString( "http://cad-comic.com/comics/%1.jpg" )
                                .arg( d->mUsedDate.toString( "yyyyMMdd" ) ) );
        } else
            url = KUrl( QString( "http://cad-comic.com/comics/20080704.jpg" ) );


        QRegExp expPrev( "/comic\\.php\\?d=" + patternDate + "\"><img src=\"/_common/images/comicnav/back1\\.jpg" );
        const int posPrev = expPrev.indexIn( data );

        if ( posPrev > -1 )
            d->mPreviousDate = QDate::fromString( expPrev.cap( 1 ), "yyyyMMdd" );

        QRegExp expNext( "/comic\\.php\\?d=" + patternDate + "\"><img src=\"/_common/images/comicnav/next1\\.jpg" );
        const int posNext = expNext.indexIn( data );

        if ( posNext > -1 )
            d->mNextDate = QDate::fromString( expNext.cap( 1 ), "yyyyMMdd" );

        requestPage( url, Private::ImageRequest );

    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void CadProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "cadprovider.moc"
