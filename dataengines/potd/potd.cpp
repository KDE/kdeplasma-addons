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

#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>
#include <QThreadPool>
#include <QDebug>

#include <KPluginLoader>
#include <KPluginMetaData>
#include <Plasma/DataContainer>

#include "cachedprovider.h"

namespace {
namespace DataKeys {
inline QString image() { return QStringLiteral("Image"); }
inline QString url()   { return QStringLiteral("Url"); }
}
}

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

    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("potd"), [](const KPluginMetaData & md) {
        return md.serviceTypes().contains(QStringLiteral("PlasmaPoTD/Plugin"));
    });

    for (const auto &metadata : plugins) {
        QString provider = metadata.value(QLatin1String( "X-KDE-PlasmaPoTDProvider-Identifier" ));
        if (provider.isEmpty()) {
            continue;
        }
        mFactories.insert(provider, metadata);
        setData( QLatin1String( "Providers" ), provider, metadata.name() );
    }
}

PotdEngine::~PotdEngine()
{
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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QStringList parts = identifier.split( QLatin1Char( ':' ), QString::SkipEmptyParts );
#else
    const QStringList parts = identifier.split( QLatin1Char( ':' ), Qt::SkipEmptyParts );
#endif
    if (parts.empty()) {
        qDebug() << "invalid identifier";
        return false;
    }
    const QString providerName = parts[ 0 ];
    if ( !mFactories.contains( providerName ) ) {
        qDebug() << "invalid provider: " << parts[ 0 ];
        return false;
    }
    
    QVariantList args;

    for (int i = 0; i < parts.count(); i++) {
        args << parts[i];
    }

    auto factory = KPluginLoader(mFactories[ providerName ].fileName()).factory();
    PotdProvider *provider = nullptr;
    if (factory) {
        provider = factory->create<PotdProvider>(this, args);
    }
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
        setData(identifier, DataKeys::image(), QImage());
        return true;
    }

    return false;
}

void PotdEngine::finished( PotdProvider *provider )
{
    if ( m_canDiscardCache && qobject_cast<CachedProvider *>( provider ) ) {
        Plasma::DataContainer *source = containerForSource( provider->identifier() );
        if ( source && !source->data().value(DataKeys::image()).value<QImage>().isNull() ) {
            provider->deleteLater();
            return;
        }
    }

    QImage img(provider->image());
    // store in cache if it's not the response of a CachedProvider
    if ( qobject_cast<CachedProvider*>( provider ) == nullptr && !img.isNull() ) {
        SaveImageThread *thread = new SaveImageThread( provider->identifier(), img );
        connect(thread, SIGNAL(done(QString,QString,QImage)), this, SLOT(cachingFinished(QString,QString,QImage)));
        QThreadPool::globalInstance()->start(thread);
    } else {
        setData(provider->identifier(), DataKeys::image(), img);
        setData(provider->identifier(), DataKeys::url(), CachedProvider::identifierToPath( provider->identifier()));
    }

    provider->deleteLater();
}

void PotdEngine::cachingFinished( const QString &source, const QString &path, const QImage &img )
{
    setData(source, DataKeys::image(), img);
    setData(source, DataKeys::url(), path);
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
    QRegularExpression re(QLatin1String(":\\d{4}-\\d{2}-\\d{2}"));

    while ( it.hasNext() ) {
        it.next();

        if (it.key() == QLatin1String("Providers")) {
            continue;
        }

        // Check if the identifier contains ISO date string, like 2019-01-09.
        // If so, don't update the picture. Otherwise, update the picture.
        if ( !re.match(it.key()).hasMatch() ) {
            const QString path = CachedProvider::identifierToPath( it.key() );
            if ( !QFile::exists(path) ) {
                updateSourceEvent( it.key() );
            } else {
                QFileInfo info( path );
                if ( info.lastModified().daysTo( QDateTime::currentDateTime() ) >= 1 ) {
                    updateSourceEvent( it.key() );
                }
            }
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(potdengine, PotdEngine, "plasma-dataengine-potd.json")

#include "potd.moc"
