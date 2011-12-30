/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
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

#include "potd.h"

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QThreadPool>

#include <KDebug>
#include <KServiceTypeTrader>

#include <Plasma/DataContainer>

#include "cachedprovider.h"
#include "kstandarddirs.h"

PotdEngine::PotdEngine( QObject* parent, const QVariantList& args )
    : Plasma::DataEngine( parent, args )
{
    // set polling to every 5 minutes
    setMinimumPollingInterval(5 * 60 * 1000);
    m_checkDatesTimer = new QTimer( this );//change picture after 24 hours
    connect( m_checkDatesTimer, SIGNAL(timeout()), this, SLOT(checkDayChanged()) );
    //FIXME: would be nice to stop and start this timer ONLY as needed, e.g. only when there are
    // time insensitive sources to serve; still, this is better than how i found it, checking
    // every 2 seconds (!)
    m_checkDatesTimer->setInterval( 10 * 60 * 1000 ); // check every 10 minutes
    m_checkDatesTimer->start();
}

PotdEngine::~PotdEngine()
{
}

void PotdEngine::init()
{
    KService::List services = KServiceTypeTrader::self()->query(QLatin1String( "PlasmaPoTD/Plugin" ));
    Q_FOREACH ( const KService::Ptr &service, services ) {
        QString provider = service->property(QLatin1String( "X-KDE-PlasmaPoTDProvider-Identifier" ), QVariant::String).toString();
        mFactories.insert(provider, service);
        setData( QLatin1String( "Providers" ), provider, service->name() );
    }
}

bool PotdEngine::updateSourceEvent( const QString &identifier )
{
    return updateSource( identifier, false );
}

bool PotdEngine::updateSource( const QString &identifier, bool loadCachedAlways )
{
    // check whether it is cached already...
    if ( CachedProvider::isCached( identifier, loadCachedAlways ) ) {
        QVariantList args;
        args << QLatin1String( "String" ) << identifier;

        CachedProvider *provider = new CachedProvider( identifier, this );
        connect( provider, SIGNAL(finished(PotdProvider*)), this, SLOT(finished(PotdProvider*)) );
        connect( provider, SIGNAL(error(PotdProvider*)), this, SLOT(error(PotdProvider*)) );

        m_canDiscardCache = loadCachedAlways;
        if (!loadCachedAlways) {
            return true;
        }
    }

    const QStringList parts = identifier.split( QLatin1Char( ':' ), QString::SkipEmptyParts );
    const QString providerName = parts[ 0 ];
    if ( !mFactories.contains( providerName ) ) {
        kDebug() << "invalid provider: " << parts[ 0 ];
        return false;
    }

    QVariantList args;
    args << providerName;
    if ( parts.count() > 1 ) {
        const QDate date = QDate::fromString( parts[ 1 ], Qt::ISODate );
        if ( !date.isValid() ) {
            kDebug() << "invalid date:" << parts[1];
            return false;
        }

        args << date;
    }

    PotdProvider *provider = qobject_cast<PotdProvider*>( mFactories[ providerName ]->createInstance<QObject>( this, args ) );
    if (provider) {
        connect( provider, SIGNAL(finished(PotdProvider*)), this, SLOT(finished(PotdProvider*)) );
        connect( provider, SIGNAL(error(PotdProvider*)), this, SLOT(error(PotdProvider*)) );
        return true;
    }

    return false;
}

bool PotdEngine::sourceRequestEvent( const QString &identifier )
{
    if ( updateSource( identifier, true ) ) {
        setData( identifier, "Image", QImage() );
        return true;
    }

    return false;
}

void PotdEngine::finished( PotdProvider *provider )
{
    if ( m_canDiscardCache && qobject_cast<CachedProvider *>( provider ) ) {
        Plasma::DataContainer *source = containerForSource( provider->identifier() );
        if ( source && !source->data().value( "Image" ).value<QImage>().isNull() ) {
            provider->deleteLater();
            return;
        }
    }

    QImage img(provider->image());
    // store in cache if it's not the response of a CachedProvider
    if ( qobject_cast<CachedProvider*>( provider ) == 0 && !img.isNull() ) {
        SaveImageThread *thread = new SaveImageThread( provider->identifier(), img );
        connect(thread, SIGNAL(done(QString,QString,QImage)), this, SLOT(cachingFinished(QString,QString,QImage)));
        QThreadPool::globalInstance()->start(thread);
    } else {
        setData( provider->identifier(), "Image", img );
        setData( provider->identifier(), "Url", CachedProvider::identifierToPath( provider->identifier()) );
    }

    provider->deleteLater();
}

void PotdEngine::cachingFinished( const QString &source, const QString &path, const QImage &img )
{
    setData( source, "Image", img );
    setData( source, "Url", path );
}

void PotdEngine::error( PotdProvider *provider )
{
    provider->disconnect(this);
    provider->deleteLater();
}

void PotdEngine::checkDayChanged()
{
    SourceDict dict = containerDict();
    QHashIterator<QString, Plasma::DataContainer*> it( dict );
    while ( it.hasNext() ) {
        it.next();

        if ( it.key() == "Providers" ) {
            continue;
        }

        if ( !it.key().contains(':') ) {
            const QString path = CachedProvider::identifierToPath( it.key() );
            if ( !QFile::exists(path) ) {
                updateSourceEvent( it.key() );
            } else {
                QFileInfo info( path );
                if ( info.lastModified().daysTo( QDateTime::currentDateTime() ) > 1 ) {
                    updateSourceEvent( it.key() );
                }
            }
        }
    }
}


#include "potd.moc"
