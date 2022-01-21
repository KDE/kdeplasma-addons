/*
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdprovidermodel.h"

#include <chrono>

#include <QDate>
#include <QDBusConnection>
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths> // For "Pictures" folder
#include <QThreadPool>

#include <KIO/CopyJob> // For "Save Image"
#include <KLocalizedString>
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

    connect(&m_checkDatesTimer, &QTimer::timeout, this, &PotdProviderModel::forceUpdateSource);
    m_checkDatesTimer.setInterval(10min); // check every 10 minutes

    qRegisterMetaType<PotdProviderData>();
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
        QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                             QStringLiteral("/org/freedesktop/login1"),
                                             QStringLiteral("org.freedesktop.login1.Manager"),
                                             QStringLiteral("PrepareForSleep"),
                                             this,
                                             SLOT(slotPrepareForSleep(bool)));
    } else {
        m_checkDatesTimer.stop();
        QDBusConnection::systemBus().disconnect(QStringLiteral("org.freedesktop.login1"),
                                                QStringLiteral("/org/freedesktop/login1"),
                                                QStringLiteral("org.freedesktop.login1.Manager"),
                                                QStringLiteral("PrepareForSleep"),
                                                this,
                                                SLOT(slotPrepareForSleep(bool)));
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
    if (const QString path = CachedProvider::identifierToPath(m_identifier); QFile::exists(path)) {
        setImage(QImage(path));
    } else {
        resetData();
    }
    updateSource();

    Q_EMIT identifierChanged();
}

QVariantList PotdProviderModel::arguments() const
{
    return m_args;
}

void PotdProviderModel::setArguments(const QVariantList &args)
{
    if (args.isEmpty() || m_args == args) {
        return;
    }

    m_args = args;

    // Avoid flickering
    if (const QString path = CachedProvider::identifierToPath(m_identifier); QFile::exists(path)) {
        setImage(QImage(path));
    } else {
        resetData();
    }
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

bool PotdProviderModel::loading() const
{
    return m_loading;
}

void PotdProviderModel::setLoading(bool status)
{
    if (m_loading == status) {
        return;
    }

    m_loading = status;
    Q_EMIT loadingChanged();
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

QUrl PotdProviderModel::infoUrl() const
{
    return m_data.wallpaperInfoUrl;
}

void PotdProviderModel::setInfoUrl(const QUrl &url)
{
    if (m_data.wallpaperInfoUrl == url) {
        return;
    }

    m_data.wallpaperInfoUrl = url;
    Q_EMIT infoUrlChanged();
}

QUrl PotdProviderModel::remoteUrl() const
{
    return m_data.wallpaperRemoteUrl;
}

void PotdProviderModel::setRemoteUrl(const QUrl &url)
{
    if (m_data.wallpaperRemoteUrl == url) {
        return;
    }

    m_data.wallpaperRemoteUrl = url;
    Q_EMIT remoteUrlChanged();
}

QString PotdProviderModel::title() const
{
    return m_data.wallpaperTitle;
}

void PotdProviderModel::setTitle(const QString &title)
{
    if (m_data.wallpaperTitle == title) {
        return;
    }

    m_data.wallpaperTitle = title;
    Q_EMIT titleChanged();
}

QString PotdProviderModel::author() const
{
    return m_data.wallpaperAuthor;
}

void PotdProviderModel::setAuthor(const QString &author)
{
    if (m_data.wallpaperAuthor == author) {
        return;
    }

    m_data.wallpaperAuthor = author;
    Q_EMIT authorChanged();
}

void PotdProviderModel::saveImage()
{
    if (m_data.wallpaperLocalUrl.isEmpty()) {
        return;
    }

    auto sanitizeFileName = [](const QString &name){
        if (name.isEmpty()) {
            return name;
        }

        const char notAllowedChars[] = ",^@={}[]~!?:&*\"|#%<>$\"'();`'/\\";
        QString sanitizedName(name);

        for (const char *c = notAllowedChars; *c; c++) {
            sanitizedName.replace(QLatin1Char(*c), QLatin1Char('-'));
        }

        return sanitizedName;
    };

    const QStringList &locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    const QString path = locations.isEmpty() ? QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0) : locations.at(0);

    // clang-format off
    QString defaultFileName = m_providers.at(m_currentIndex).name().trimmed();

    if (!m_data.wallpaperTitle.isEmpty()) {
        defaultFileName += QLatin1Char('-') + m_data.wallpaperTitle.trimmed();
        if (!m_data.wallpaperAuthor.isEmpty()) {
            defaultFileName += QLatin1Char('-') + m_data.wallpaperAuthor.trimmed();
        }
    } else {
        // Use current date
        if (!defaultFileName.isEmpty()) {
            defaultFileName += QLatin1Char('-');
        }
        defaultFileName += QDate::currentDate().toString();
    }

    m_savedUrl = QUrl::fromLocalFile(
        QFileDialog::getSaveFileName(
            nullptr,
            i18nc("@title:window", "Save Today's Picture"),
            path + "/" + sanitizeFileName(defaultFileName) + ".jpg",
            i18nc("@label:listbox Template for file dialog", "JPEG image (*.jpeg *.jpg *.jpe)"),
            nullptr,
            QFileDialog::DontConfirmOverwrite // KIO::CopyJob will show the confirmation dialog.
        )
    );
    // clang-format on

    if (m_savedUrl.isEmpty() || !m_savedUrl.isValid()) {
        return;
    }

    m_savedFolder = QUrl::fromLocalFile(m_savedUrl.toLocalFile().section(QDir::separator(), 0, -2));

    KIO::CopyJob *copyJob = KIO::copy(QUrl::fromLocalFile(m_data.wallpaperLocalUrl), m_savedUrl, KIO::HideProgressInfo);
    connect(copyJob, &KJob::finished, this, [this](KJob *job) {
        if (job->error()) {
            m_saveStatusMessage = job->errorText();
            if (m_saveStatusMessage.isEmpty()) {
                m_saveStatusMessage = i18nc("@info:status after a save action", "The image was not saved.");
            }
            m_saveStatus = FileOperationStatus::Failed;
            Q_EMIT saveStatusChanged();
        } else {
            m_saveStatusMessage = i18nc("@info:status after a save action %1 file path %2 basename",
                                         "The image was saved as <a href=\"%1\">%2</a>",
                                         m_savedUrl.toString(),
                                         m_savedUrl.fileName());
            m_saveStatus = FileOperationStatus::Successful;
            Q_EMIT saveStatusChanged();
        }
    });
    copyJob->start();
}

void PotdProviderModel::resetData()
{
    setImage(QImage());
    setLocalUrl(QString());
    setInfoUrl(QUrl());
    setRemoteUrl(QUrl());
    setTitle(QString());
    setAuthor(QString());
}

bool PotdProviderModel::updateSource(bool refresh)
{
    setLoading(true);

    // Check whether it is cached already...
    if (!refresh && CachedProvider::isCached(m_identifier, false)) {
        CachedProvider *provider = new CachedProvider(m_identifier, this);
        connect(provider, &PotdProvider::finished, this, &PotdProviderModel::slotFinished);
        connect(provider, &PotdProvider::error, this, &PotdProviderModel::slotError);
        setLoading(false);
        return true;
    }

    if (m_currentIndex < 0) {
        qCWarning(WALLPAPERPOTD) << "Invalid provider: " << m_identifier;
        setLoading(false);
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

    setInfoUrl(provider->infoUrl());
    setRemoteUrl(provider->remoteUrl());
    setTitle(provider->title());
    setAuthor(provider->author());

    // Store in cache if it's not the response of a CachedProvider
    if (qobject_cast<CachedProvider *>(provider) == nullptr) {
        setImage(provider->image());
        setLoading(false);
        SaveImageThread *thread = new SaveImageThread(m_identifier, m_data);
        connect(thread, &SaveImageThread::done, this, &PotdProviderModel::slotCachingFinished);
        QThreadPool::globalInstance()->start(thread);
    } else {
        // Image is loaded in setIdentifier or setArguments
        setLocalUrl(CachedProvider::identifierToPath(m_identifier));
    }

    // Do not update until next day, and delay 1s to make sure last modified condition is satisfied.
    if (running()) {
        m_checkDatesTimer.setInterval(QDateTime::currentDateTime().msecsTo(QDate::currentDate().startOfDay().addDays(1)) + 1000);
        m_checkDatesTimer.start();
    }

    provider->deleteLater();
}

void PotdProviderModel::slotCachingFinished(const QString &source, const PotdProviderData &data)
{
    Q_UNUSED(source)
    setImage(data.wallpaperImage);
    setLocalUrl(data.wallpaperLocalUrl);
    setInfoUrl(data.wallpaperInfoUrl);
    setRemoteUrl(data.wallpaperRemoteUrl);
    setTitle(data.wallpaperTitle);
    setAuthor(data.wallpaperAuthor);
}

void PotdProviderModel::slotError(PotdProvider *provider)
{
    provider->disconnect(this);
    provider->deleteLater();

    setLoading(false);

    // Retry 10min later
    if (running()) {
        m_checkDatesTimer.setInterval(10min);
        m_checkDatesTimer.start();
    }
}

void PotdProviderModel::slotPrepareForSleep(bool sleep)
{
    if (!sleep) {
        forceUpdateSource();
    }
}
