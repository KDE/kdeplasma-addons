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
    QString name;
    QDate date;
    QString identifier;
};

PotdProvider::PotdProvider(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new PotdProviderPrivate)
{
    if (args.count() > 0) {
        d->name = args[0].toString();

        d->identifier = d->name;

        if (args.count() > 1) {
            for (int i = 1; i < args.count(); i++) {
                d->identifier += QStringLiteral(":") + args[i].toString();
                QDate date = QDate::fromString(args[i].toString(), Qt::ISODate);
                if (date.isValid()) {
                    d->date = date;
                }
            }
        }
    } else {
        d->name = QStringLiteral("Unknown");
        d->identifier = d->name;
    }

    QString configFileName = d->identifier + QStringLiteral("provider.conf");
    configRemoteUrl = QUrl(QStringLiteral(CONFIG_ROOT_URL) + configFileName);
    configLocalPath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/plasma_engine_potd/") + configFileName;
    configLocalUrl = QUrl::fromLocalFile(configLocalPath);
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
