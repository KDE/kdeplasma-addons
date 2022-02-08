// SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
// SPDX-FileCopyrightText: 2021 Guo Yunhe <i@guoyunhe.me>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "potdprovider.h"

#include <QDate>
#include <QDebug>
#include <QFileInfo>

#include <KConfig>
#include <KConfigGroup>

#define CONFIG_ROOT_URL "https://autoconfig.kde.org/potd/"

class PotdProviderPrivate
{
public:
    explicit PotdProviderPrivate();
    ~PotdProviderPrivate();

    QString name;
    QDate date;
    QString identifier;

    std::unique_ptr<PotdProviderData> m_data;
};

PotdProviderPrivate::PotdProviderPrivate()
    : m_data(std::make_unique<PotdProviderData>())
{
}

PotdProviderPrivate::~PotdProviderPrivate()
{
}

PotdProvider::PotdProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : QObject(parent)
    , d(new PotdProviderPrivate)
{
    if (const QString name = data.name(); !name.isEmpty()) {
        d->name = name;
    } else {
        d->name = QStringLiteral("Unknown");
    }

    if (const QString identifier = data.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")); !identifier.isEmpty()) {
        d->identifier = identifier;
    } else {
        d->identifier = d->name;
    }

    if (!args.empty()) {
        for (const auto &arg : args) {
            const QDate date = QDate::fromString(arg.toString(), Qt::ISODate);
            if (date.isValid()) {
                d->date = date;
                break;
            }
        }
    }

    QString configFileName = d->identifier + QStringLiteral("provider.conf");
    configRemoteUrl = QUrl(QStringLiteral(CONFIG_ROOT_URL) + configFileName);
    configLocalPath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/plasma_engine_potd/") + configFileName;
    configLocalUrl = QUrl::fromLocalFile(configLocalPath);
}

PotdProvider::PotdProvider(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new PotdProviderPrivate)
{
    Q_UNUSED(args)
    d->name = QStringLiteral("Unknown");
    d->identifier = d->name;
    qWarning() << "You are using an old PoTD provider plugin. It will not work in Plasma 6. Please consider updating the plugin.";
}

PotdProvider::~PotdProvider()
{
}

QString PotdProvider::name() const
{
    return d->name;
}

QDate PotdProvider::date() const
{
    return d->date.isNull() ? QDate::currentDate() : d->date;
}

bool PotdProvider::isFixedDate() const
{
    return !d->date.isNull();
}

QString PotdProvider::identifier() const
{
    return d->identifier;
}

QImage PotdProvider::image() const
{
    return potdProviderData()->wallpaperImage;
}

QUrl PotdProvider::remoteUrl() const
{
    return potdProviderData()->wallpaperRemoteUrl;
}

QUrl PotdProvider::infoUrl() const
{
    return potdProviderData()->wallpaperInfoUrl;
}

QString PotdProvider::title() const
{
    return potdProviderData()->wallpaperTitle;
}

QString PotdProvider::author() const
{
    return potdProviderData()->wallpaperAuthor;
}

void PotdProvider::refreshConfig()
{
    // You can only refresh it once in a provider's life cycle
    if (refreshed) {
        return;
    }
    // You can only refresh it once in a day
    QFileInfo configFileInfo = QFileInfo(configLocalPath);
    if (configFileInfo.exists() && configFileInfo.lastModified().addDays(1) > QDateTime::currentDateTime()) {
        return;
    }

    KIO::StoredTransferJob *job = KIO::storedGet(configRemoteUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &PotdProvider::configRequestFinished);

    refreshed = true;
}

void PotdProvider::configRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        qDebug() << "configRequestFinished error: failed to fetch data";
        return;
    }

    KIO::StoredTransferJob *putJob = KIO::storedPut(job->data(), configLocalUrl, -1);
    connect(putJob, &KIO::StoredTransferJob::finished, this, &PotdProvider::configWriteFinished);
}

void PotdProvider::configWriteFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        qDebug() << "configWriteFinished error: failed to write data";
        return;
    }

    loadConfig();
}

void PotdProvider::loadConfig()
{
    KConfig config(configLocalPath);

    KConfigGroup apiGroup = config.group("API");
    QString apiKey = apiGroup.readEntry("API_KEY");
    QString apiSecret = apiGroup.readEntry("API_SECRET");

    Q_EMIT configLoaded(apiKey, apiSecret);
}

PotdProviderData *PotdProvider::potdProviderData() const
{
    return d->m_data.get();
}
