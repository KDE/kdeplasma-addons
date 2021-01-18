/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "wcpotdprovider.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrlQuery>

#include <KIO/Job>
#include <KPluginFactory>

WcpotdProvider::WcpotdProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    QUrl url(QStringLiteral("https://commons.wikimedia.org/w/api.php"));

    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("action"), QStringLiteral("parse"));
    urlQuery.addQueryItem(QStringLiteral("text"), QStringLiteral("{{Potd}}"));
    urlQuery.addQueryItem(QStringLiteral("contentmodel"), QStringLiteral("wikitext"));
    urlQuery.addQueryItem(QStringLiteral("prop"), QStringLiteral("images"));
    urlQuery.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    url.setQuery(urlQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &WcpotdProvider::pageRequestFinished);
}

WcpotdProvider::~WcpotdProvider() = default;

QImage WcpotdProvider::image() const
{
    return mImage;
}

void WcpotdProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        emit error(this);
        return;
    }

    auto jsonImageArray = QJsonDocument::fromJson(job->data()).object().value(QLatin1String("parse")).toObject().value(QLatin1String("images")).toArray();

    if (jsonImageArray.size() > 0) {
        const QString imageFile = jsonImageArray.at(0).toString();
        if (!imageFile.isEmpty()) {
            const QUrl picUrl(QLatin1String("https://commons.wikimedia.org/wiki/Special:FilePath/") + imageFile);
            KIO::StoredTransferJob *imageJob = KIO::storedGet(picUrl, KIO::NoReload, KIO::HideProgressInfo);
            connect(imageJob, &KIO::StoredTransferJob::finished, this, &WcpotdProvider::imageRequestFinished);
            return;
        }
    }

    emit error(this);
}

void WcpotdProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        emit error(this);
        return;
    }
    QByteArray data = job->data();
    mImage = QImage::fromData(data);
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(WcpotdProvider, "wcpotdprovider.json")

#include "wcpotdprovider.moc"
