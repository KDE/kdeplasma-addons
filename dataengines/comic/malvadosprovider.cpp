/*
*   Copyright (C) 2008 Hugo Parente Lima <hugo.pl@gmail.com>
*   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>
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

#include "malvadosprovider.h"

#include <QtGui/QImage>

COMICPROVIDER_EXPORT_PLUGIN( MalvadosProvider, "MalvadosProvider", "" )

class MalvadosProvider::Private
{
    public:
        enum RequestType
        {
            PageRequest,
            ImageRequest,
            PageSubRequest
        };

        Private()
            : mComicId( 0 ), mMaxId( 0 )
        {
        }

        QImage mImage;
        int mComicId;
        int mMaxId;
};

MalvadosProvider::MalvadosProvider( QObject* parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setComicAuthor( "André Dahmer" );
    d->mComicId = requestedNumber();

    setWebsiteHttp();
}

void MalvadosProvider::setWebsiteHttp()
{
    KUrl url( QString( "http://www.malvados.com.br/" ) );

    if ( d->mMaxId ) {
        url.setPath( QString( "/index%1.html" ).arg( d->mComicId ) );
    } else {
        url.setPath( "/index.html" );
    }

    requestPage( url, Private::PageRequest );
}

MalvadosProvider::~MalvadosProvider()
{
    delete d;
}

ComicProvider::IdentifierType MalvadosProvider::identifierType() const
{
    return NumberIdentifier;
}

KUrl MalvadosProvider::websiteUrl() const
{
    return QString( "http://www.malvados.com.br/index%1.html" ).arg( d->mComicId );
}

QImage MalvadosProvider::image() const
{
    return d->mImage;
}

QString MalvadosProvider::identifier() const
{
    return QString( "malvados:%1" ).arg( d->mComicId );
}

QString MalvadosProvider::nextIdentifier() const
{
    return ( ( d->mComicId < d->mMaxId ) ? QString::number( d->mComicId + 1 ) : QString() );
}

QString MalvadosProvider::previousIdentifier() const
{
    return ( ( d->mComicId > firstStripNumber() ) ? QString::number( d->mComicId - 1 ) : QString() );
}

void MalvadosProvider::pageRetrieved( int id, const QByteArray &rawData )
{
    if ( ( id == Private::PageRequest ) || ( id == Private::PageSubRequest ) ) {
        const QString data = QString::fromLatin1( rawData );

        if ( !d->mMaxId ) {
            QString pattern;

            if ( id == Private::PageRequest ) {
                pattern = QString( "<frame name=\"mainFrame\" src=\"index(\\d+).html\">" );
            } else if ( id == Private::PageSubRequest ) {
                pattern = QString( "index(\\d+)" );
            }

            QRegExp exp( pattern );
            const int pos = exp.indexIn( data );

            if ( pos > -1 ) {
                d->mMaxId = exp.cap( 1 ).toInt();
            // no infinite calls!
            } else if ( id == Private::PageSubRequest ) {
                emit error( this );
            // handles the case when the author did not publish a comic ( is drunk etc. )
            } else {
                const QString pattern( "<frame name=\"mainFrame\" src=\"(.+\\.html)\">" );
                QRegExp exp( pattern );
                const int pos = exp.indexIn( data );
                if ( pos > -1 ) {
                    KUrl url( QString( "http://www.malvados.com.br/%1" ).arg( exp.cap( 1 ) ) );
                    requestPage( url, Private::PageSubRequest );
                    return;
                } else {
                    emit error( this );
                }
            }
            if ( !d->mComicId ) {
                d->mComicId = d->mMaxId;
            }

            setWebsiteHttp();
            return;
        }

        const QString pattern( "<div align=\"center\">.*<img src=\"(.+)\"" );
        QRegExp exp ( pattern );
        exp.setMinimal( true );
        const int pos = exp.indexIn( data );

        if ( pos > -1 ) {
            KUrl url( QString( "http://www.malvados.com.br/%1" )
            .arg( exp.cap( 1 ) ) );

            requestPage( url, Private::ImageRequest );
        }
    } else if ( id == Private::ImageRequest ) {
        d->mImage = QImage::fromData( rawData );
        emit finished( this );
    }
}

void MalvadosProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "malvadosprovider.moc"
