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

#include "doonesburyprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( DoonesburyProvider, "DoonesburyProvider", "" )

class DoonesburyProvider::Private
{
    public:
        QImage mImage;
};

DoonesburyProvider::DoonesburyProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setComicAuthor( "Garry Trudeau" );
    setFirstStripDate( QDate( 1970, 10, 26 ) );
    KUrl url( QString( "http://picayune.uclick.com/comics/db/%1/db%2.gif" ).arg( requestedDate().toString( "yyyy" ) )
                                                                          .arg( requestedDate().toString( "yyMMdd" ) ) );

    requestPage( url, 0 );
}

DoonesburyProvider::~DoonesburyProvider()
{
    delete d;
}

ComicProvider::IdentifierType DoonesburyProvider::identifierType() const
{
    return DateIdentifier;
}

QImage DoonesburyProvider::image() const
{
    return d->mImage;
}

QString DoonesburyProvider::identifier() const
{
    return QString( "doonesbury:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl DoonesburyProvider::websiteUrl() const
{
    return QString( "http://www.gocomics.com/doonesbury/%1/" ).arg( requestedDate().toString( "yyyy/MM/dd" ) );
}

void DoonesburyProvider::pageRetrieved( int, const QByteArray &data )
{
    d->mImage = QImage::fromData( data );

    emit finished( this );
}

void DoonesburyProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "doonesburyprovider.moc"
