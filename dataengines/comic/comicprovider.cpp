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

#include "comicprovider.h"

#include <QtNetwork/QHttp>

class ComicProvider::Private
{
    public:
        Private( ComicProvider *parent )
            : mParent( parent ),
              mIsCurrent( false )
        {
        }

        void jobDone( bool error )
        {
            QHttp *http = qobject_cast<QHttp*>( mParent->sender() );
            if ( !http )
                return;

            if ( error ) {
                mParent->pageError( http->property( "uid" ).toInt(), http->errorString() );
            } else {
                mParent->pageRetrieved( http->property( "uid" ).toInt(), http->readAll() );
            }

            http->deleteLater();
        }

        ComicProvider *mParent;
        QDate mRequestedDate;
        int mRequestedNumber;
        QString mRequestedId;
        bool mIsCurrent;
};

ComicProvider::ComicProvider( QObject *parent, const QVariantList &args )
    : QObject( parent ), d( new Private( this ) )
{
    Q_ASSERT( args.count() == 2 );

    const QString type = args[ 0 ].toString();
    if ( type == "Date" )
        d->mRequestedDate = args[ 1 ].toDate();
    else if ( type == "Number" )
        d->mRequestedNumber = args[ 1 ].toInt();
    else if ( type == "String" )
        d->mRequestedId = args[ 1 ].toString();
    else
        Q_ASSERT( false && "Invalid type passed to comic provider" );
}

ComicProvider::~ComicProvider()
{
    delete d;
}

QString ComicProvider::nextIdentifier() const
{
    if ( identifierType() == DateIdentifier && d->mRequestedDate != QDate::currentDate() )
        return d->mRequestedDate.addDays( 1 ).toString( Qt::ISODate );

    return QString();
}

QString ComicProvider::previousIdentifier() const
{
    if ( identifierType() == DateIdentifier )
        return d->mRequestedDate.addDays( -1 ).toString( Qt::ISODate );

    return QString();
}

void ComicProvider::setIsCurrent( bool value )
{
    d->mIsCurrent = value;
}

bool ComicProvider::isCurrent() const
{
    return d->mIsCurrent;
}

QDate ComicProvider::requestedDate() const
{
    return d->mRequestedDate;
}

int ComicProvider::requestedNumber() const
{
    return d->mRequestedNumber;
}

QString ComicProvider::requestedString() const
{
    return d->mRequestedId;
}

void ComicProvider::requestPage( const QString &host, int port, const QString &path, int id, const MetaInfos &infos )
{
    QHttp *http = new QHttp( host, port, this );
    http->setProperty( "uid", id );
    connect( http, SIGNAL( done( bool ) ), this, SLOT( jobDone( bool ) ) );

    if ( infos.isEmpty() ) {
        http->get( path );
    } else {
        QHttpRequestHeader header( "GET", path );
        QMapIterator<QString, QString> it( infos );
        while ( it.hasNext() ) {
            it.next();
            header.setValue( it.key(), it.value() );
        }

        http->request( header );
    }
}

void ComicProvider::pageRetrieved( int, const QByteArray& )
{
}

void ComicProvider::pageError( int, const QString& )
{
}

#include "comicprovider.moc"
