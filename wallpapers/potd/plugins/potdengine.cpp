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

namespace
{
#if HAVE_NetworkManagerQt

bool isUsingMeteredConnection()
{
    const auto metered = NetworkManager::metered();
    return metered == NetworkManager::Device::MeteredStatus::GuessYes //
        || metered == NetworkManager::Device::MeteredStatus::Yes;
}
#endif
}

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
    if (m_loading) {
        return;
    }

    setLoading(true);

    // Check whether it is cached already...
#if HAVE_NetworkManagerQt
    // Use cache even if it's outdated when using metered connection
    const bool ignoreAge = m_doesUpdateOverMeteredConnection == 0 && isUsingMeteredConnection();
    if (!refresh && (CachedProvider::isCached(m_identifier, m_args, ignoreAge))) {
#else
    if (!refresh && CachedProvider::isCached(m_identifier, m_args, false)) {
#endif
        qCDebug(WALLPAPERPOTD) << "A local cache is available for" << m_identifier << "with arguments" << m_args;

        CachedProvider *provider = new CachedProvider(m_identifier, m_args, this);
        connect(provider, &PotdProvider::finished, this, &PotdClient::slotFinished);
        connect(provider, &PotdProvider::error, this, &PotdClient::slotError);
        return;
    }

    // Find the old cached image and load it for now
    if (m_data.wallpaperImage.isNull()) {
        const QString path = CachedProvider::identifierToPath(m_identifier, m_args);
        if (QFile::exists(path)) {
            setImage(QImage(path));
        }
    }

#if HAVE_NetworkManagerQt
    if (m_doesUpdateOverMeteredConnection == 0 && isUsingMeteredConnection()) {
        qCDebug(WALLPAPERPOTD) << "Skip updating wallpapers for" << m_identifier << m_args << "due to metered connection.";
        setLoading(false);
        Q_EMIT done(this, false);
        return;
    }
#endif

    const auto pluginResult = KPluginFactory::instantiatePlugin<PotdProvider>(m_metadata, this, m_args);

    if (pluginResult) {
        qCDebug(WALLPAPERPOTD) << "Downloading wallpaper from" << m_identifier << m_args;
        connect(pluginResult.plugin, &PotdProvider::finished, this, &PotdClient::slotFinished);
        connect(pluginResult.plugin, &PotdProvider::error, this, &PotdClient::slotError);
    } else {
        qCWarning(WALLPAPERPOTD) << "Error loading PoTD plugin:" << pluginResult.errorString;
    }
}

#if HAVE_NetworkManagerQt
void PotdClient::setUpdateOverMeteredConnection(int value)
{
    // Don't return if values are the same because there can be multiple
    // backends. Instead, let updateSource() decide whether to update
    // the wallpaper.

    m_doesUpdateOverMeteredConnection = value;
    if (m_doesUpdateOverMeteredConnection == 1 && isUsingMeteredConnection()) {
        updateSource();
    }
}
#endif

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
    Q_EMIT done(this, true);
}

void PotdClient::slotError(PotdProvider *provider)
{
    qCWarning(WALLPAPERPOTD) << m_identifier << "with arguments" << m_args
                             << "failed to fetch the remote wallpaper. Please check your Internet connection or system date.";
    provider->deleteLater();
    setLoading(false);
    Q_EMIT done(this, false);
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
{
    loadPluginMetaData();

    connect(&m_checkDatesTimer, &QTimer::timeout, this, &PotdEngine::forceUpdateSource);

    int interval = QDateTime::currentDateTime().msecsTo(QDate::currentDate().addDays(1).startOfDay()) + 1000;
    m_checkDatesTimer.setInterval(std::max(interval, 60 * 1000));
    m_checkDatesTimer.start();
    qCDebug(WALLPAPERPOTD) << "Time to next update (h):" << m_checkDatesTimer.interval() / 1000.0 / 60.0 / 60.0;

    // Sleep checker
    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(slotPrepareForSleep(bool)));

#if HAVE_NetworkManagerQt
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &PotdEngine::slotConnectivityChanged);
#endif
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

        qCDebug(WALLPAPERPOTD) << identifier << "is registered with arguments" << args;
        auto client = new PotdClient(pluginIt->second, args, this);
        m_clientMap.emplace(identifier, ClientPair{client, 1});

        return client;
    };

    while (pr.first != pr.second) {
        // find exact match
        if (pr.first->second.client->m_args == args) {
            pr.first->second.instanceCount++;
            qCDebug(WALLPAPERPOTD) << identifier << "is registered with arguments" << args << "Total client(s):" << pr.first->second.instanceCount;
            return pr.first->second.client;
        }

        pr.first++;
    }

    return createClient();
}

void PotdEngine::unregisterClient(const QString &identifier, const QVariantList &args)
{
    auto [beginIt, endIt] = m_clientMap.equal_range(identifier);

    while (beginIt != endIt) {
        // find exact match
        if (beginIt->second.client->m_args == args) {
            beginIt->second.instanceCount--;
            qCDebug(WALLPAPERPOTD) << identifier << "with arguments" << args << "is unregistered. Remaining client(s):" << beginIt->second.instanceCount;
            if (!beginIt->second.instanceCount) {
                delete beginIt->second.client;
                m_clientMap.erase(beginIt);
                qCDebug(WALLPAPERPOTD) << identifier << "with arguments" << args << "is freed.";
                break;
            }
        }

        beginIt++;
    }
}

void PotdEngine::updateSource(bool refresh)
{
    m_lastUpdateSuccess = true;

    for (const auto &[_, clientPair] : std::as_const(m_clientMap)) {
        if (clientPair.client->m_loading) {
            continue;
        }

        connect(clientPair.client, &PotdClient::done, this, &PotdEngine::slotDone);
        m_updateCount++;
        qCDebug(WALLPAPERPOTD) << clientPair.client->m_metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")) << "starts updating wallpaper.";
        clientPair.client->updateSource(refresh);
    }
}

void PotdEngine::forceUpdateSource()
{
    updateSource(true);
}

void PotdEngine::slotDone(PotdClient *client, bool success)
{
    disconnect(client, &PotdClient::done, this, &PotdEngine::slotDone);

    qCDebug(WALLPAPERPOTD) << client->m_identifier << "with arguments" << client->m_args << (success ? "finished" : "failed")
                           << "updating the wallpaper. Remaining clients:" << m_updateCount - 1;

    if (!success) {
        m_lastUpdateSuccess = false;
    }

    if (!--m_updateCount) {
        // Do not update until next day, and delay 1s to make sure last modified condition is satisfied.
        if (m_lastUpdateSuccess) {
            m_checkDatesTimer.setInterval(std::max<int>(QDateTime::currentDateTime().msecsTo(QDate::currentDate().startOfDay().addDays(1)) + 1000, 60000));
        } else {
            m_checkDatesTimer.setInterval(10min);
        }
        m_checkDatesTimer.start();
        qCDebug(WALLPAPERPOTD) << "Time to next update (h):" << m_checkDatesTimer.interval() / 1000.0 / 60.0 / 60.0;
    }
}

void PotdEngine::slotPrepareForSleep(bool sleep)
{
    if (sleep) {
        return;
    }

#if HAVE_NetworkManagerQt
    if (NetworkManager::connectivity() != NetworkManager::Connectivity::Full) {
        qCDebug(WALLPAPERPOTD) << "Waking up from sleep but the network is not connected yet.";
        return;
    }
#endif

    // Resume from sleep
    // Always force update to work around the current date not being updated
    forceUpdateSource();
}

#if HAVE_NetworkManagerQt
void PotdEngine::slotConnectivityChanged(NetworkManager::Connectivity connectivity)
{
    if (connectivity == NetworkManager::Connectivity::Full) {
        qCDebug(WALLPAPERPOTD) << "Network is connected.";
        updateSource(false);
    }
}
#endif

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
