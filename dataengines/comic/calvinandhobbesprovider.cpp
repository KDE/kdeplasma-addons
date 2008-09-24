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
#include <QtGui/QImage>

#include <KUrl>

#include "calvinandhobbesprovider.h"

COMICPROVIDER_EXPORT_PLUGIN( CalvinAndHobbesProvider, "CalvinAndHobbesProvider", "" )

class CalvinAndHobbesProvider::Private
{
    public:
        QImage mImage;
};


CalvinAndHobbesProvider::CalvinAndHobbesProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args ), d( new Private )
{
    setComicAuthor( "Bill Watterson" );
    setFirstStripDate( QDate( 1985, 11, 17 ) );
    KUrl url( QString( "http://picayune.uclick.com/comics/ch/%1/ch%2.gif" ).arg( requestedDate().toString( "yyyy" ) )
                                                                          .arg( requestedDate().toString( "yyMMdd" ) ) );

    requestPage( url, 0 );
}

CalvinAndHobbesProvider::~CalvinAndHobbesProvider()
{
    delete d;
}

ComicProvider::IdentifierType CalvinAndHobbesProvider::identifierType() const
{
    return DateIdentifier;
}

QImage CalvinAndHobbesProvider::image() const
{
    return d->mImage;
}

QString CalvinAndHobbesProvider::identifier() const
{
    return QString( "calvinandhobbes:%1" ).arg( requestedDate().toString( Qt::ISODate ) );
}

KUrl CalvinAndHobbesProvider::websiteUrl() const
{
    return QString( "http://www.gocomics.com/calvinandhobbes/%1/" ).arg( requestedDate().toString( "yyyy/MM/dd" ) );
}

void CalvinAndHobbesProvider::pageRetrieved( int, const QByteArray &data )
{
    d->mImage = QImage::fromData( data );
    emit finished( this );
}

void CalvinAndHobbesProvider::pageError( int, const QString& )
{
    emit error( this );
}

#include "calvinandhobbesprovider.moc"
