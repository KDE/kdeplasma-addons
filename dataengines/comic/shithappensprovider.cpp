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

#include <QtCore/QRegExp>
#include <QtGui/QPainter>
#include <QtGui/QImage>

#include <KUrl>
#include <KStandardDirs>

#include "shithappensprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( ShitHappensProvider, "ShitHappensProvider", "" )

class ShitHappensProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest
        };
        Private()
            : mGotMaxId ( false )
        {
        }

        QImage mImage;
        int mRequestedId;
        int mMaxId;
        int mModifiedId;
        bool mGotMaxId;
};


ShitHappensProvider::ShitHappensProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    d->mRequestedId = requestedNumber();

    setWebsiteHttp();
}

void ShitHappensProvider::setWebsiteHttp()
{
    KUrl url( QString( "http://ruthe.de/" ) );

    if ( d->mGotMaxId ) {
        if ( ( d->mRequestedId < 1 ) || ( d->mRequestedId > d->mMaxId ) )
            d->mRequestedId = d->mMaxId;

        d->mModifiedId = d->mMaxId - d->mRequestedId;

        url.setPath( QString( "/gallery/cpg1410/displayimage.php?album=4&pos=%1" )
                            .arg( d->mModifiedId ) );
    } else {
        url.setPath( QString( "/gallery/cpg1410/displayimage.php?album=4&pos=0" ) );
    }

    requestPage( "ruthe.de", 80, url.path(), Private::PageRequest );
}

ShitHappensProvider::~ShitHappensProvider()
{
    delete d;
}

ComicProvider::IdentifierType ShitHappensProvider::identifierType() const
{
    return NumberIdentifier;
}

QImage ShitHappensProvider::image() const
{
    return d->mImage;
}

QString ShitHappensProvider::identifier() const
{
    return QString( "shithappens:%1" ).arg( d->mRequestedId );
}

KUrl ShitHappensProvider::websiteUrl() const
{
    return QString( "http://www.ruthe.de/" );
}

QString ShitHappensProvider::nextIdentifier() const
{
    if ( d->mRequestedId < d->mMaxId )
        return QString::number( d->mRequestedId + 1 );

    return QString();
}

QString ShitHappensProvider::previousIdentifier() const
{
    if ( d->mRequestedId > 1 )
        return QString::number( d->mRequestedId - 1 );

    return QString();
}

void ShitHappensProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( id == Private::PageRequest ) {
        const QString data = QString::fromUtf8( rawData );

        KUrl url;

        if ( !d->mGotMaxId ) {
            QRegExp expMaxId( "\\b\\w+\\b (\\d+)/(\\d+)" );
            const int pos = expMaxId.indexIn( data );

            if ( pos > -1 ) {
                d->mMaxId = expMaxId.cap( 2 ).toInt();
                d->mGotMaxId = true;
                setWebsiteHttp();
                return;
            } else {
                emit error( this );
                return;
            }
        } else {
            QRegExp exp( "<img src=\"albums/userpics/(\\d+)/normal_strip_(\\d+)(\\S*\\d*)\\.jpg\"" );
            const int pos = exp.indexIn( data );

            if ( pos > -1 ) {
                url = KUrl( QString( "http://ruthe.de/gallery/cpg1410/albums/userpics/%1/strip_%2%3.jpg" )
                                    .arg( exp.cap( 1 ) ).arg( exp.cap( 2 ) ).arg( exp.cap( 3 ) ) );
            } else {
                url = KUrl( QString( "http://ruthe.de/bilder/shit_happens_header.gif" ) );
            }
        }

        requestPage( "ruthe.de", 80, url.path(), Private::ImageRequest );
    } else if ( id == Private::ImageRequest ) {
        const QString headerRelLoc( "plasma-comic/plasma_comic_shithappens-header.png" );
        const QImage header( KStandardDirs::locate( "data", headerRelLoc ) );
        const QImage comic = QImage::fromData( rawData );

        const int spaceTop = 8;
        const int spaceMid = 20;
        const int spaceBot = 13;

        const int height = header.height() + comic.height() + spaceTop + spaceMid + spaceBot;
        const int width = (header.width() >= comic.width()) ? header.width() : comic.width();

        d->mImage = QImage( QSize( width, height ), QImage::Format_RGB32 );
        d->mImage.fill( header.pixel( QPoint( 0, 0 ) ) );

        QPainter painter( &(d->mImage) );

        // center and draw the Images
        const QPoint headerPos( ( ( width - header.width() ) / 2 ), spaceTop );
        const QPoint comicPos( ( ( width - comic.width() ) / 2 ), spaceTop + header.height() + spaceMid );
        painter.drawImage( headerPos, header );
        painter.drawImage( comicPos, comic );

        emit finished( this );
    }
}

void ShitHappensProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "shithappensprovider.moc"
