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

#include "nichtlustigprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( NichtLustigProvider, "NichtLustigProvider", "" )

class NichtLustigProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };

        Private( NichtLustigProvider *parent )
            : mUsedDate( parent->requestedDate() )
        {
        }

        QImage mImage;
        QDate mUsedDate;
        bool mFindNewDate;
        QDate mPreviousDate;
        QDate mNextDate;
};


NichtLustigProvider::NichtLustigProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private( this ) )
{
    d->mFindNewDate = (d->mUsedDate == QDate::currentDate());

    setWebsiteHttp();
}

void NichtLustigProvider::setWebsiteHttp()
{
    KUrl url( QString( "http://nicht-lustig.de/" ) );

    if ( d->mFindNewDate )
        url.setPath( QString( "/main.html" ) );
    else
        url.setPath( QString( "/toondb/%1.html" ).arg( d->mUsedDate.toString( "yyMMdd" ) ) );

    requestPage( "nicht-lustig.de", 80, url.path(), Private::PageRequest );
}

NichtLustigProvider::~NichtLustigProvider()
{
    delete d;
}

ComicProvider::IdentifierType NichtLustigProvider::identifierType() const
{
    return DateIdentifier;
}

QImage NichtLustigProvider::image() const
{
    return d->mImage;
}

QString NichtLustigProvider::identifier() const
{
    return QString( "nichtlustig:%1" ).arg( d->mUsedDate.toString( Qt::ISODate ) );
}

KUrl NichtLustigProvider::websiteUrl() const
{
    return QString( "http://nicht-lustig.de/toondb/%1.html" ).arg( d->mUsedDate.toString( "yyMMdd" ) );
}

QString NichtLustigProvider::nextIdentifier() const
{
    return d->mNextDate.toString( Qt::ISODate );
}

QString NichtLustigProvider::previousIdentifier() const
{
    return d->mPreviousDate.toString( Qt::ISODate );
}

void NichtLustigProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString data = QString::fromUtf8( rawData );
        const QString patternDate( "((\\d\\d)(0[1-9]|1[012])(0[1-9]|[12][0-9]|3[01]))" );

        QRegExp exp( "(" + patternDate + "\\.jpg)" );
        const int pos = exp.indexIn( data );

        KUrl url;

        if ( pos > -1 ) {
            if ( d->mFindNewDate ) {
                d->mUsedDate =  QDate::fromString( "20" + exp.cap( 2 ), "yyyyMMdd" );
                d->mFindNewDate = false;
                setWebsiteHttp();
                return;
            }

            QString sub = exp.cap( 1 );
            url = KUrl( QString( "http://nicht-lustig.de/comics/full/%1.jpg" )
                                .arg( d->mUsedDate.toString( "yyMMdd" ) ) );
        }

        QRegExp expPrev( "id=\"links\"><a href=\"" + patternDate +  "\\.html\"" );
        const int posPrev = expPrev.indexIn( data );

        if ( posPrev > -1 )
            d->mPreviousDate = QDate::fromString( "20" + expPrev.cap( 1 ), "yyyyMMdd" );

        QRegExp expNext( patternDate + "\\.html\"><img src=\"/sys_img/toon/pfeil_rechts_normal\\.gif" );
        const int posNext = expNext.indexIn( data );

        if ( posNext > -1 )
            d->mNextDate = QDate::fromString( "20" + expNext.cap( 1 ), "yyyyMMdd" );

        requestPage( "nicht-lustig.de", 80, url.path(), Private::ImageRequest );

    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void NichtLustigProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "nichtlustigprovider.moc"
