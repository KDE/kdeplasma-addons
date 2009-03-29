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

#include <KDebug>
#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KUrl>

class ComicProvider::Private
{
    public:
        Private( KService::Ptr service, ComicProvider *parent )
            : mParent( parent ),
              mIsCurrent( false ),
              mFirstStripNumber( 1 ),
              mComicDescription( service )
        {
        }

        void jobDone( KJob *job )
        {
            if ( job->error() ) {
                mParent->pageError( job->property( "uid" ).toInt(), job->errorText() );
            } else {
                KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>( job );
                mParent->pageRetrieved( job->property( "uid" ).toInt(), storedJob->data() );
            }
        }

        ComicProvider *mParent;
        QString mRequestedId;
        QString mRequestedComicName;
        QString mComicAuthor;
        bool mIsCurrent;
        bool mIsLeftToRight;
        bool mIsTopToBottom;
        QDate mRequestedDate;
        QDate mFirstStripDate;
        int mRequestedNumber;
        int mFirstStripNumber;
        KPluginInfo mComicDescription;
};

ComicProvider::ComicProvider( QObject *parent, const QVariantList &args )
    : QObject( parent ), d( new Private(
      KService::serviceByStorageId( args.count() > 2 ? args[2].toString() : QString() ), this ) )
{
    Q_ASSERT( args.count() >= 2 );
    const QString type = args[ 0 ].toString();

    if ( type == "Date" )
        d->mRequestedDate = args[ 1 ].toDate();
    else if ( type == "Number" )
        d->mRequestedNumber = args[ 1 ].toInt();
    else if ( type == "String" ) {
        d->mRequestedId = args[ 1 ].toString();

        int index = d->mRequestedId.indexOf( ':' );
        d->mRequestedComicName = d->mRequestedId.mid( 0, index );
    }
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
    if ( ( identifierType() == DateIdentifier ) && ( !firstStripDate().isValid() || d->mRequestedDate > firstStripDate() ) )
        return d->mRequestedDate.addDays( -1 ).toString( Qt::ISODate );

    return QString();
}

QString ComicProvider::stripTitle() const
{
    return QString();
}

QString ComicProvider::additionalText() const
{
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

QDate ComicProvider::firstStripDate() const
{
    return d->mFirstStripDate;
}

QString ComicProvider::comicAuthor() const
{
    return d->mComicAuthor;
}

void ComicProvider::setComicAuthor( const QString &author )
{
    d->mComicAuthor = author;
}

void ComicProvider::setFirstStripDate( const QDate &date )
{
    d->mFirstStripDate = date;
}

int ComicProvider::firstStripNumber() const
{
    return d->mFirstStripNumber;
}

void ComicProvider::setFirstStripNumber( int number )
{
    d->mFirstStripNumber = number;
}

QString ComicProvider::firstStripIdentifier() const
{
    if ( ( identifierType() == DateIdentifier ) && d->mFirstStripDate.isValid() ) {
        return d->mFirstStripDate.toString( Qt::ISODate );
    } else if ( identifierType() == NumberIdentifier ) {
        return QString::number( d->mFirstStripNumber );
    }

    return QString();
}

int ComicProvider::requestedNumber() const
{
    return d->mRequestedNumber;
}

QString ComicProvider::requestedString() const
{
    return d->mRequestedId;
}

QString ComicProvider::requestedComicName() const
{
    return d->mRequestedComicName;
}

void ComicProvider::requestPage( const KUrl &url, int id, const MetaInfos &infos )
{
    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    job->setProperty( "uid", id );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( jobDone( KJob* ) ) );

    if ( !infos.isEmpty() ) {
        QMapIterator<QString, QString> it( infos );
        while ( it.hasNext() ) {
            it.next();
            job->addMetaData( it.key(), it.value() );
        }
    }
}

void ComicProvider::pageRetrieved( int, const QByteArray& )
{
}

void ComicProvider::pageError( int, const QString& )
{
}

QString ComicProvider::pluginName() const
{
    if ( !d->mComicDescription.isValid() ) {
        return QString();
    }
    return d->mComicDescription.pluginName();
}

QString ComicProvider::name() const
{
    if ( !d->mComicDescription.isValid() ) {
        return QString();
    }
    return d->mComicDescription.name();
}

QString ComicProvider::suffixType() const
{
    if ( !d->mComicDescription.isValid() ) {
        return QString();
    }
    return d->mComicDescription.property( "X-KDE-PlasmaComicProvider-SuffixType" ).toString();
}

KPluginInfo ComicProvider::description() const
{
    return d->mComicDescription;
}

KUrl ComicProvider::shopUrl() const
{
    return KUrl();
}

bool ComicProvider::isLeftToRight() const
{
    return true;
}

bool ComicProvider::isTopToBottom() const
{
    return true;
}

#include "comicprovider.moc"
