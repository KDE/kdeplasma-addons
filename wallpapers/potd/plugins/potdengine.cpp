/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdengine.h"

#include <chrono>

#include <QDBusConnection>
#include <QThreadPool>

#include <KPluginFactory>

#include "cachedprovider.h"
#include "debug.h"

using namespace std::chrono_literals;

PotdClient::PotdClient(const KPluginMetaData &metadata, const QVariantList &args, QObject *parent)
    : QObject(parent)
    , m_metadata(metadata)
    , m_identifier(metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")))
    , m_args(args)
{
    updateSource();
}

void PotdClient::updateSource(bool refresh)
{
    setLoading(true);

    // Check whether it is cached already...
    if (!refresh && CachedProvider::isCached(m_identifier, m_args, false)) {
        CachedProvider *provider = new CachedProvider(m_identifier, m_args, this);
        connect(provider, &PotdProvider::finished, this, &PotdClient::slotFinished);
        connect(provider, &PotdProvider::error, this, &PotdClient::slotError);
        return;
    }

    const auto pluginResult = KPluginFactory::instantiatePlugin<PotdProvider>(m_metadata, this, m_args);

    if (pluginResult) {
        connect(pluginResult.plugin, &PotdProvider::finished, this, &PotdClient::slotFinished);
        connect(pluginResult.plugin, &PotdProvider::error, this, &PotdClient::slotError);
    } else {
        qCWarning(WALLPAPERPOTD) << "Error loading PoTD plugin:" << pluginResult.errorString;
    }
}

void PotdClient::slotFinished(PotdProvider *provider)
{
    setImage(provider->image());
    setInfoUrl(provider->infoUrl());
    setRemoteUrl(provider->remoteUrl());
    setTitle(provider->title());
    setAuthor(provider->author());

    // Store in cache if it's not the response of a CachedProvider
    if (qobject_cast<CachedProvider *>(provider) == nullptr) {
        SaveImageThread *thread = new SaveImageThread(m_identifier, m_args, m_data);
        connect(thread, &SaveImageThread::done, this, &PotdClient::slotCachingFinished);
        QThreadPool::globalInstance()->start(thread);
    } else {
        // Is cache provider
        setLocalUrl(CachedProvider::identifierToPath(m_identifier, m_args));
    }

    provider->deleteLater();
    setLoading(false);
    Q_EMIT done(this);
}

void PotdClient::slotError(PotdProvider *provider)
{
    provider->deleteLater();
    setLoading(false);
    Q_EMIT done(this);
}

void PotdClient::slotCachingFinished(const QString &, const PotdProviderData &data)
{
    setLocalUrl(data.wallpaperLocalUrl);
}

void PotdClient::setImage(const QImage &image)
{
    m_data.wallpaperImage = image;
    Q_EMIT imageChanged();
}

void PotdClient::setLoading(bool status)
{
    if (status == m_loading) {
        return;
    }

    m_loading = status;
    Q_EMIT loadingChanged();
}

void PotdClient::setLocalUrl(const QString &urlString)
{
    if (m_data.wallpaperLocalUrl == urlString) {
        return;
    }

    m_data.wallpaperLocalUrl = urlString;
    Q_EMIT localUrlChanged();
}

void PotdClient::setInfoUrl(const QUrl &url)
{
    if (m_data.wallpaperInfoUrl == url) {
        return;
    }

    m_data.wallpaperInfoUrl = url;
    Q_EMIT infoUrlChanged();
}

void PotdClient::setRemoteUrl(const QUrl &url)
{
    if (m_data.wallpaperRemoteUrl == url) {
        return;
    }

    m_data.wallpaperRemoteUrl = url;
    Q_EMIT remoteUrlChanged();
}

void PotdClient::setTitle(const QString &title)
{
    if (m_data.wallpaperTitle == title) {
        return;
    }

    m_data.wallpaperTitle = title;
    Q_EMIT titleChanged();
}

void PotdClient::setAuthor(const QString &author)
{
    if (m_data.wallpaperAuthor == author) {
        return;
    }

    m_data.wallpaperAuthor = author;
    Q_EMIT authorChanged();
}

PotdEngine::PotdEngine(QObject *parent)
    : QObject(parent)
    , m_lastUpdateDate(QDate::currentDate().addDays(-1))
{
    loadPluginMetaData();

    connect(&m_checkDatesTimer, &QTimer::timeout, this, &PotdEngine::forceUpdateSource);

    int interval = QDateTime::currentDateTime().msecsTo(QDate::currentDate().addDays(1).startOfDay()) + 1000;
    m_checkDatesTimer.setInterval(std::max(interval, 60 * 1000));
    m_checkDatesTimer.start();

    // Sleep checker
    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(slotPrepareForSleep(bool)));
}

PotdClient *PotdEngine::registerClient(const QString &identifier, const QVariantList &args)
{
    auto pr = m_clientMap.equal_range(identifier);

    auto createClient = [this, &identifier, &args]() -> PotdClient * {
        auto pluginIt = m_providersMap.find(identifier);

        if (pluginIt == m_providersMap.end()) {
            // Not a valid identifier
            return nullptr;
        }

        auto client = new PotdClient(pluginIt->second, args, this);
        m_clientMap.emplace(identifier, ClientPair{client, 1});

        return client;
    };

    if (pr.first == pr.second) {
        return createClient();
    }

    while (pr.first != pr.second) {
        // find exact match
        if (pr.first->second.client->m_args == args) {
            pr.first->second.instanceCount++;
            return pr.first->second.client;
        }

        pr.first++;
    }

    return createClient();
}

void PotdEngine::unregisterClient(const QString &identifier, const QVariantList &args)
{
    auto pr = m_clientMap.equal_range(identifier);

    while (pr.first != pr.second) {
        // find exact match
        if (pr.first->second.client->m_args == args) {
            if (!--pr.first->second.instanceCount) {
                delete pr.first->second.client;
                m_clientMap.erase(pr.first);
                break;
            }
        }

        pr.first++;
    }
}

void PotdEngine::updateSource(bool refresh)
{
    for (const auto &pr : std::as_const(m_clientMap)) {
        connect(pr.second.client, &PotdClient::done, this, &PotdEngine::slotDone);
        pr.second.client->updateSource(refresh);
        m_updateCount++;
    }
}

void PotdEngine::forceUpdateSource()
{
    updateSource(true);
}

void PotdEngine::slotDone(PotdClient *client)
{
    disconnect(client, &PotdClient::done, this, &PotdEngine::slotDone);

    if (!--m_updateCount) {
        // Do not update until next day, and delay 1s to make sure last modified condition is satisfied.
        m_lastUpdateDate = QDate::currentDate();
        m_checkDatesTimer.setInterval(QDateTime::currentDateTime().msecsTo(m_lastUpdateDate.startOfDay().addDays(1)) + 1000);
        m_checkDatesTimer.start();
    }
}

void PotdEngine::slotPrepareForSleep(bool sleep)
{
    if (sleep) {
        return;
    }

    // Resume from sleep
    if (m_lastUpdateDate != QDate::currentDate()) {
        // New day new wallpaper
        forceUpdateSource();
    } else {
        // Align the update timer's interval, and delay 1s to make sure last modified condition is satisfied.
        const int remainingTime = QDateTime::currentDateTime().msecsTo(m_lastUpdateDate.addDays(1).startOfDay()) + 1000;

        // In case the remaining time is too short, set the interval to 1min
        m_checkDatesTimer.setInterval(std::max(remainingTime, 60 * 1000));
        m_checkDatesTimer.start();
    }
}

void PotdEngine::loadPluginMetaData()
{
    const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("potd"));

    m_providersMap.clear();
    m_providersMap.reserve(plugins.size());

    for (const KPluginMetaData &metadata : plugins) {
        const QString identifier = metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier"));
        if (!identifier.isEmpty()) {
            m_providersMap.emplace(identifier, metadata);
        }
    }
}
