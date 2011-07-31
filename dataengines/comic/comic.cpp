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

#include "comic.h"

#include <QtCore/QDate>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

#include <KDebug>
#include <KServiceTypeTrader>
#include <KSycoca>
#include <KStandardDirs>

#include "cachedprovider.h"

ComicEngine::ComicEngine( QObject* parent, const QVariantList& args )
    : Plasma::DataEngine( parent, args ), mEmptySuffix( false )
{
    setPollingInterval( 0 );
    updateFactories();
}

ComicEngine::~ComicEngine()
{
}

void ComicEngine::init()
{
    connect( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
             this, SLOT(networkStatusChanged(Solid::Networking::Status)) );
    connect( KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(sycocaUpdated(QStringList)) );
}

void ComicEngine::networkStatusChanged( Solid::Networking::Status status )
{
    if ( ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown ) &&
         !mIdentifierError.isEmpty() ) {
        sourceRequestEvent( mIdentifierError );
    }
}

void ComicEngine::sycocaUpdated( const QStringList &changedResources )
{
    if ( changedResources.contains( QLatin1String( "services" ) ) ) {
        updateFactories();
    }
}

void ComicEngine::updateFactories()
{
    mFactories.clear();
    removeAllData( QLatin1String( "providers" ) );
    KService::List services = KServiceTypeTrader::self()->query( QLatin1String( "Plasma/Comic" ) );
    Q_FOREACH ( const KService::Ptr &service, services ) {
        mFactories.insert( service->property( QLatin1String( "X-KDE-PluginInfo-Name" ), QVariant::String ).toString(),
                           service );
        if ( service->isDeleted() ) {
            continue;
        }
        QStringList data;
        data << service->name();
        QFileInfo file( service->icon() );
        if ( file.isRelative() ) {
            data << KStandardDirs::locate( "data", QString( QLatin1String( "plasma-comic/%1.png" ) ).arg( service->icon() ) );
        } else {
            data << service->icon();
        }
        setData( QLatin1String( "providers" ), service->property( QLatin1String( "X-KDE-PluginInfo-Name" ), QVariant::String ).toString(), data );
    }
}

bool ComicEngine::updateSourceEvent( const QString &identifier )
{
    if ( identifier == QLatin1String( "providers" ) ) {
        updateFactories();
        return true;
    } else if ( identifier.startsWith( QLatin1String( "setting_maxComicLimit:" ) ) ) {
        bool worked;
        const int maxComicLimit = identifier.mid( 22 ).toInt( &worked );
        if ( worked ) {
            CachedProvider::setMaxComicLimit( maxComicLimit );
        }
        return worked;
    } else {
        if ( m_jobs.contains(identifier) ) {
            return true;
        }

        // check whether it is cached already...
        if ( CachedProvider::isCached( identifier ) ) {
            QVariantList args;
            args << QLatin1String( "String" ) << identifier;

            ComicProvider *provider = new CachedProvider( this, args );
            m_jobs[identifier] = provider;
            connect( provider, SIGNAL(finished(ComicProvider*)), this, SLOT(finished(ComicProvider*)) );
            connect( provider, SIGNAL(error(ComicProvider*)), this, SLOT(error(ComicProvider*)) );
            return true;
        }

        // ... start a new query otherwise
        const QStringList parts = identifier.split( QLatin1Char( ':' ), QString::KeepEmptyParts );

        //: are mandatory
        if ( parts.count() < 2 ) {
            setData( identifier, QLatin1String( "Error" ), true );
            kError() << "Less than two arguments specified.";
            return false;
        }
        if ( !mFactories.contains( parts[ 0 ] ) ) {
            // User might have installed more from GHNS
            updateFactories();
            if ( !mFactories.contains( parts[ 0 ] ) ) {
                setData( identifier, QLatin1String( "Error" ), true );
                kError() << identifier << "comic plugin does not seem to be installed.";
                return false;
            }
        }

        // check if there is a connection
        Solid::Networking::Status status = Solid::Networking::status();
        if ( status != Solid::Networking::Connected && status != Solid::Networking::Unknown ) {
            mIdentifierError = identifier;
            setData( identifier, QLatin1String( "Error" ), true );
            setData( identifier, QLatin1String( "Error automatically fixable" ), true );
            setData( identifier, QLatin1String( "Identifier" ), identifier );
            setData( identifier, QLatin1String( "Previous identifier suffix" ), lastCachedIdentifier( identifier ) );
            kWarning() << "No connection.";
            return true;
        }

        const KService::Ptr service = mFactories[ parts[ 0 ] ];

        bool isCurrentComic = parts[ 1 ].isEmpty();

        QVariantList args;
        ComicProvider *provider = 0;

        const QString type = service->property( QLatin1String( "X-KDE-PlasmaComicProvider-SuffixType" ), QVariant::String ).toString();
        if ( type == QLatin1String( "Date" ) ) {
            QDate date = QDate::fromString( parts[ 1 ], Qt::ISODate );
            if ( !date.isValid() )
                date = QDate::currentDate();

            args << QLatin1String( "Date" ) << date;
        } else if ( type == QLatin1String( "Number" ) ) {
            args << QLatin1String( "Number" ) << parts[ 1 ].toInt();
        } else if ( type == QLatin1String( "String" ) ) {
            args << QLatin1String( "String" ) << parts[ 1 ];
        }
        args << service->storageId();

        provider = service->createInstance<ComicProvider>( this, args );
        if ( !provider ) {
            setData( identifier, QLatin1String( "Error" ), true );
            kError() << identifier << "plugin could be created.";
            return false;
        }
        provider->setIsCurrent( isCurrentComic );

        m_jobs[identifier] = provider;

        connect( provider, SIGNAL(finished(ComicProvider*)), this, SLOT(finished(ComicProvider*)) );
        connect( provider, SIGNAL(error(ComicProvider*)), this, SLOT(error(ComicProvider*)) );
        return true;
    }
}

bool ComicEngine::sourceRequestEvent( const QString &identifier )
{
    setData( identifier, DataEngine::Data() );

    return updateSourceEvent( identifier );
}

void ComicEngine::finished( ComicProvider *provider )
{
    // sets the data
    setComicData( provider );
    if ( provider->image().isNull() ) {
        error( provider );
        return;
    }

    // different comic -- with no error yet -- has been chosen, old error is invalidated
    QString temp = mIdentifierError.left( mIdentifierError.indexOf( QLatin1Char( ':' ) ) + 1 );
    if ( !mIdentifierError.isEmpty() && provider->identifier().indexOf( temp ) == -1 ) {
        mIdentifierError.clear();
    }
    // comic strip with error worked now
    if ( !mIdentifierError.isEmpty() && ( mIdentifierError == provider->identifier() ) ){
        mIdentifierError.clear();
    }

    // store in cache if it's not the response of a CachedProvider,
    // if there is a valid image and if there is a next comic
    // (if we're on today's comic it could become stale)
    if ( !provider->inherits("CachedProvider") && !provider->image().isNull() &&
         !provider->nextIdentifier().isEmpty() ) {
        CachedProvider::Settings info;

        info[ QLatin1String( "websiteUrl" ) ] = provider->websiteUrl().prettyUrl();
        info[ QLatin1String( "imageUrl" ) ] = provider->imageUrl().url();
        info[ QLatin1String( "shopUrl" ) ] = provider->shopUrl().prettyUrl();
        info[ QLatin1String( "nextIdentifier" ) ] = provider->nextIdentifier();
        info[ QLatin1String( "previousIdentifier" ) ] = provider->previousIdentifier();
        info[ QLatin1String( "title" ) ] = provider->name();
        info[ QLatin1String( "suffixType" ) ] = provider->suffixType();
        info[ QLatin1String( "lastCachedStripIdentifier" ) ] = provider->identifier().mid( provider->identifier().indexOf( QLatin1Char( ':' ) ) + 1 );
        QString isLeftToRight;
        QString isTopToBottom;
        info[ QLatin1String( "isLeftToRight" ) ] = isLeftToRight.setNum( provider->isLeftToRight() );
        info[ QLatin1String( "isTopToBottom" ) ] = isTopToBottom.setNum( provider->isTopToBottom() );

        //data that should be only written if available
        if ( !provider->comicAuthor().isEmpty() ) {
            info[ QLatin1String( "comicAuthor" ) ] = provider->comicAuthor();
        }
        if ( !provider->firstStripIdentifier().isEmpty() ) {
            info[ QLatin1String( "firstStripIdentifier" ) ] = provider->firstStripIdentifier();
        }
        if ( !provider->additionalText().isEmpty() ) {
            info[ QLatin1String( "additionalText" ) ] = provider->additionalText();
        }
        if ( !provider->stripTitle().isEmpty() ) {
            info[ QLatin1String( "stripTitle" ) ] = provider->stripTitle();
        }

        CachedProvider::storeInCache( provider->identifier(), provider->image(), info );
    }
    provider->deleteLater();

    const QString key = m_jobs.key(provider);
    if (!key.isEmpty()) {
        m_jobs.remove(key);
    }
}

void ComicEngine::error( ComicProvider *provider )
{
    // sets the data
    setComicData( provider );

    QString identifier( provider->identifier() );
    mIdentifierError = identifier;

    kWarning() << identifier << "pluging reported an error.";

    /**
     * Requests for the current day have no suffix (date or id)
     * set initially, so we have to remove the 'faked' suffix
     * here again to not confuse the applet.
     */
    if ( provider->isCurrent() )
        identifier = identifier.left( identifier.indexOf( QLatin1Char( ':' ) ) + 1 );

    setData( identifier, QLatin1String( "Identifier" ), identifier );
    setData( identifier, QLatin1String( "Error" ), true );

    // if there was an error loading the last cached comic strip, do not return its id anymore
    const QString lastCachedId = lastCachedIdentifier( identifier );
    if ( lastCachedId != provider->identifier().mid( provider->identifier().indexOf( QLatin1Char( ':' ) ) + 1 ) ) {
        // sets the previousIdentifier to the identifier of a strip that has been cached before
        setData( identifier, QLatin1String( "Previous identifier suffix" ), lastCachedId );
    }
    setData( identifier, QLatin1String( "Next identifier suffix" ), QString() );

    const QString key = m_jobs.key(provider);
    if (!key.isEmpty()) {
        m_jobs.remove(key);
    }

    provider->deleteLater();
}

void ComicEngine::setComicData( ComicProvider *provider )
{
    QString identifier( provider->identifier() );

    /**
     * Requests for the current day have no suffix (date or id)
     * set initially, so we have to remove the 'faked' suffix
     * here again to not confuse the applet.
     */
    if ( provider->isCurrent() )
        identifier = identifier.left( identifier.indexOf( QLatin1Char( ':' ) ) + 1 );

    setData( identifier, QLatin1String( "Image" ), provider->image() );
    setData( identifier, QLatin1String( "Website Url" ), provider->websiteUrl() );
    setData( identifier, QLatin1String( "Image Url" ), provider->imageUrl() );
    setData( identifier, QLatin1String( "Shop Url" ), provider->shopUrl() );
    setData( identifier, QLatin1String( "Next identifier suffix" ), provider->nextIdentifier() );
    setData( identifier, QLatin1String( "Previous identifier suffix" ), provider->previousIdentifier() );
    setData( identifier, QLatin1String( "Comic Author" ), provider->comicAuthor() );
    setData( identifier, QLatin1String( "Additional text" ), provider->additionalText() );
    setData( identifier, QLatin1String( "Strip title" ), provider->stripTitle() );
    setData( identifier, QLatin1String( "First strip identifier suffix" ), provider->firstStripIdentifier() );
    setData( identifier, QLatin1String( "Identifier" ), provider->identifier() );
    setData( identifier, QLatin1String( "Title" ), provider->name() );
    setData( identifier, QLatin1String( "SuffixType" ), provider->suffixType() );
    setData( identifier, QLatin1String( "isLeftToRight" ), provider->isLeftToRight() );
    setData( identifier, QLatin1String( "isTopToBottom" ), provider->isTopToBottom() );
    setData( identifier, QLatin1String( "Error" ), false );
}

QString ComicEngine::lastCachedIdentifier( const QString &identifier ) const
{
        const QString id = identifier.left( identifier.indexOf( QLatin1Char( ':' ) ) );
        QString data = KStandardDirs::locateLocal( "data", QLatin1String( "plasma_engine_comic/" ) );
        data += QString::fromAscii( QUrl::toPercentEncoding( id ) );
        QSettings settings( data + QLatin1String( ".conf" ), QSettings::IniFormat );
        QString previousIdentifier = settings.value( QLatin1String( "lastCachedStripIdentifier" ), QString() ).toString();

        return previousIdentifier;
}

#include "comic.moc"
