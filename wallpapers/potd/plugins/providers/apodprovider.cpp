/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "apodprovider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

ApodProvider::ApodProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    QUrl feed_url = QUrl(QStringLiteral("https://science.nasa.gov/wp-json/wp/v2/apod-basic"));

    KIO::StoredTransferJob *job = KIO::storedGet(feed_url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &ApodProvider::feedRequestFinished);
}

void ApodProvider::feedRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    QJsonParseError parse_error;
    QJsonDocument feed = QJsonDocument::fromJson(job->data(), &parse_error);

    if (parse_error.error != QJsonParseError::NoError) {
        Q_EMIT error(this);
        return;
    }

    QJsonArray items = feed.array();
    QJsonObject potd = items.at(0).toObject();

    // If the json object doesn't have the expected structure,
    // potd will be an empty json object
    if (potd.keys().size() > 0) {
        m_title = potd.value(QStringLiteral("title")).toString();
        m_author = potd.value(QStringLiteral("credit")).toString();

        QString info_url = potd.value(QStringLiteral("permalink")).toString();
        m_infoUrl = QUrl(info_url);

        QString hd_url = potd.value(QStringLiteral("hdurl")).toString();
        m_remoteUrl = QUrl(hd_url);

        KIO::StoredTransferJob *imageJob = KIO::storedGet(m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &ApodProvider::imageRequestFinished);
    } else {
        Q_EMIT error(this);
    }
}

void ApodProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    Q_EMIT finished(this, QImage::fromData(job->data()));
}

K_PLUGIN_CLASS_WITH_JSON(ApodProvider, "apodprovider.json")

#include "apodprovider.moc"
