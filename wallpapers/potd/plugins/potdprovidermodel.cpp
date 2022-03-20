/*
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdprovidermodel.h"

#include <chrono>

#include <QDate>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QThreadPool>

#include <KPluginFactory>
#include <KPluginMetaData>

#include "cachedprovider.h"
#include "debug.h"

using namespace std::chrono_literals;

PotdProviderModel::PotdProviderModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentIndex(-1)
{
    loadPluginMetaData();

    connect(&m_checkDatesTimer, &QTimer::timeout, this, &PotdProviderModel::slotCheckDayChanged);
    // FIXME: would be nice to stop and start this timer ONLY as needed, e.g. only when there are
    // time insensitive sources to serve; still, this is better than how i found it, checking
    // every 2 seconds (!)
    m_checkDatesTimer.setInterval(10min); // check every 10 minutes
}

int PotdProviderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_providers.size();
}

QVariant PotdProviderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_providers.size())) {
        return QVariant();
    }

    const KPluginMetaData &item = m_providers.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return item.name();
    case Qt::DecorationRole:
        return item.iconName();
    case Roles::Id:
        return item.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier"));
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PotdProviderModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {Qt::DecorationRole, "decoration"},
        {Roles::Id, "id"},
    };
}

void PotdProviderModel::loadPluginMetaData()
{
    const QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("potd"));

    beginResetModel();

    m_providers.clear();
    m_providers.reserve(plugins.size());

    std::copy_if(plugins.cbegin(), plugins.cend(), std::back_inserter(m_providers), [](const KPluginMetaData &metadata) {
        return !metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")).isEmpty();
    });

    endResetModel();
}

KPluginMetaData PotdProviderModel::metadata(int index) const
{
    if (index >= static_cast<int>(m_providers.size()) || index < 0) {
        return m_providers.at(0);
    }

    return m_providers.at(index);
}

int PotdProviderModel::indexOfProvider(const QString &identifier) const
{
    const auto it = std::find_if(m_providers.cbegin(), m_providers.cend(), [&identifier](const KPluginMetaData &metadata) {
        return metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")) == identifier;
    });

    if (it == m_providers.cend()) {
        return -1;
    }

    return std::distance(m_providers.cbegin(), it);
}

bool PotdProviderModel::running() const
{
    return m_checkDatesTimer.isActive();
}

void PotdProviderModel::setRunning(bool flag)
{
    if (m_checkDatesTimer.isActive() == flag) {
        return;
    }

    if (flag) {
        m_checkDatesTimer.start();
    } else {
        m_checkDatesTimer.stop();
    }

    Q_EMIT runningChanged();
}

QString PotdProviderModel::identifier() const
{
    return m_identifier;
}

void PotdProviderModel::setIdentifier(const QString &identifier)
{
    if (m_identifier == identifier) {
        return;
    }

    m_identifier = identifier;
    m_currentIndex = indexOfProvider(identifier);

    // Avoid flickering
    const bool isCached = CachedProvider::isCached(m_identifier, false);
    if (!isCached) {
        resetData();
    }
    updateSource(!isCached);

    Q_EMIT identifierChanged();
}

QVariantList PotdProviderModel::arguments() const
{
    return m_args;
}

void PotdProviderModel::setArguments(const QVariantList &args)
{
    if (m_args == args) {
        return;
    }

    m_args = args;

    resetData();
    forceUpdateSource();

    Q_EMIT argumentsChanged();
}

QImage PotdProviderModel::image() const
{
    return m_data.wallpaperImage;
}

void PotdProviderModel::setImage(const QImage &image)
{
    m_data.wallpaperImage = image;
    Q_EMIT imageChanged();
}

QString PotdProviderModel::localUrl() const
{
    return m_data.wallpaperLocalUrl;
}

void PotdProviderModel::setLocalUrl(const QString &urlString)
{
    if (m_data.wallpaperLocalUrl == urlString) {
        return;
    }

    m_data.wallpaperLocalUrl = urlString;
    Q_EMIT localUrlChanged();
}

void PotdProviderModel::resetData()
{
    setImage(QImage());
    setLocalUrl(QString());
}

bool PotdProviderModel::updateSource(bool refresh)
{
    // Check whether it is cached already...
    if (!refresh && CachedProvider::isCached(m_identifier, false)) {
        CachedProvider *provider = new CachedProvider(m_identifier, this);
        connect(provider, &PotdProvider::finished, this, &PotdProviderModel::slotFinished);
        connect(provider, &PotdProvider::error, this, &PotdProviderModel::slotError);
        return true;
    }

    if (m_currentIndex < 0) {
        qCWarning(WALLPAPERPOTD) << "Invalid provider: " << m_identifier;
        return false;
    }

    const auto pluginResult = KPluginFactory::instantiatePlugin<PotdProvider>(metadata(m_currentIndex), this, m_args);

    if (pluginResult) {
        connect(pluginResult.plugin, &PotdProvider::finished, this, &PotdProviderModel::slotFinished);
        connect(pluginResult.plugin, &PotdProvider::error, this, &PotdProviderModel::slotError);
        return true;
    }

    qCWarning(WALLPAPERPOTD) << "Error loading PoTD plugin:" << pluginResult.errorString;
    return false;
}

bool PotdProviderModel::forceUpdateSource()
{
    return updateSource(true);
}

void PotdProviderModel::slotFinished(PotdProvider *provider)
{
    if (provider->identifier() != m_identifier) {
        return;
    }

    setImage(provider->image());

    // Store in cache if it's not the response of a CachedProvider
    if (qobject_cast<CachedProvider *>(provider) == nullptr && !m_data.wallpaperImage.isNull()) {
        SaveImageThread *thread = new SaveImageThread(m_identifier, m_data.wallpaperImage);
        connect(thread, &SaveImageThread::done, this, &PotdProviderModel::slotCachingFinished);
        QThreadPool::globalInstance()->start(thread);
    } else {
        setLocalUrl(CachedProvider::identifierToPath(m_identifier));
    }

    provider->deleteLater();
}

void PotdProviderModel::slotCachingFinished(const QString &source, const QString &path, const QImage &img)
{
    Q_UNUSED(source)
    setImage(img);
    setLocalUrl(path);
}

void PotdProviderModel::slotError(PotdProvider *provider)
{
    provider->disconnect(this);
    provider->deleteLater();
}

void PotdProviderModel::slotCheckDayChanged()
{
    const QString path = CachedProvider::identifierToPath(m_identifier);

    if (!QFile::exists(path)) {
        updateSource();
    } else {
        const QFileInfo info(path);
        if (info.lastModified().daysTo(QDateTime::currentDateTime()) >= 1) {
            updateSource();
        }
    }
}
