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
#include <KDebug>
#include <KServiceTypeTrader>

#include "cachedprovider.h"
#include "kstandarddirs.h"

PotdEngine::PotdEngine( QObject* parent, const QVariantList& args )
    : Plasma::DataEngine( parent, args )
{
    // set polling to every 5 minutes
    setMinimumPollingInterval(5 * 60 * 1000);
    // FIXME: this should almost certainly be checked ONCE per day
    //        howver, it was checking every 2 seconds, so every 5 minutes
    //        might be good enough though it certainly isn't perfect either
    setPollingInterval(5 * 60 * 1000);
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
        setData(QLatin1String( "Providers" ), service->name(), provider);
    }
}

bool PotdEngine::updateSourceEvent( const QString &identifier )
{
    // check whether it is cached already...
    if ( CachedProvider::isCached( identifier ) ) {
        QVariantList args;
        args << QLatin1String( "String" ) << identifier;

        CachedProvider *provider = new CachedProvider( identifier, this, args );
        connect( provider, SIGNAL(finished(PotdProvider*)), this, SLOT(finished(PotdProvider*)) );
        connect( provider, SIGNAL(error(PotdProvider*)), this, SLOT(error(PotdProvider*)) );
        return true;
    }

    const QStringList parts = identifier.split( QLatin1Char( ':' ), QString::SkipEmptyParts );

    //: are mandatory
    if ( parts.count() < 2 ) {
        kDebug() << "less than 2 parts";
        return false;
    }

    if ( !mFactories.contains( parts[ 0 ] ) ) {
        kDebug() << "invalid provider: " << parts[ 0 ];
        return false;
    }

    const KService::Ptr service = mFactories[ parts[ 0 ] ];

    const QDate date = QDate::fromString( parts[ 1 ], Qt::ISODate );
    if ( !date.isValid() ) {
        kDebug() << "invalid date:" << parts[1];
        return false;
    }

    QVariantList args;
    PotdProvider *provider = 0;

    args << QLatin1String( "Date" ) << date;

    provider = qobject_cast<PotdProvider*>( service->createInstance<QObject>( this, args ) );

    connect( provider, SIGNAL(finished(PotdProvider*)), this, SLOT(finished(PotdProvider*)) );
    connect( provider, SIGNAL(error(PotdProvider*)), this, SLOT(error(PotdProvider*)) );
    return true;
}

bool PotdEngine::sourceRequestEvent( const QString &identifier )
{
    if ( updateSourceEvent( identifier ) ) {
        setData( identifier, QImage() );
        return true;
    }

    return false;
}

void PotdEngine::finished( PotdProvider *provider )
{
    setData( provider->identifier(), provider->image() );
    setData( provider->identifier(), "Url", CachedProvider::identifierToPath( provider->identifier()) );

    // store in cache if it's not the response of a CachedProvider
    if ( dynamic_cast<CachedProvider*>( provider ) == 0 && !provider->image().isNull() ) {
        CachedProvider::storeInCache( provider->identifier(), provider->image() );
    }

    provider->deleteLater();
}

void PotdEngine::error( PotdProvider *provider )
{
    provider->deleteLater();
}

#include "potd.moc"
