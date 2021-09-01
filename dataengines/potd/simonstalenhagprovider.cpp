/*
 *   SPDX-FileCopyrightText: 2021 Alexey Andreyev <aa13q@ya.ru>
 *
 *   SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 */

#include "simonstalenhagprovider.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRandomGenerator>

#include <KIO/Job>
#include <KPluginFactory>

static QJsonValue randomArrayValueByKey(const QJsonObject &object, QLatin1String key)
{
    QJsonValue result;

    if (object.isEmpty()) {
        return result;
    }

    auto array = object.value(key).toArray();

    if (array.isEmpty()) {
        return result;
    }

    auto arraySize = array.size();
    return array.at(QRandomGenerator::global()->bounded(arraySize));
}

SimonStalenhagProvider::SimonStalenhagProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://raw.githubusercontent.com/a-andreyev/simonstalenhag-se-metadata/main/entrypoint.json"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &SimonStalenhagProvider::entrypointRequestFinished);
}

QImage SimonStalenhagProvider::image() const
{
    return mImage;
}

void SimonStalenhagProvider::entrypointRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    auto json = QJsonDocument::fromJson(job->data());
    auto metadataString = randomArrayValueByKey(json.object(), QLatin1String("simonstalenhag-se-entrypoint"));
    auto urlStr = metadataString.toString();
    if (urlStr.isEmpty()) {
        Q_EMIT error(this);
        return;
    }
    QUrl metaDataUrl(urlStr);
    KIO::StoredTransferJob *metaDataJob = KIO::storedGet(metaDataUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(metaDataJob, &KIO::StoredTransferJob::finished, this, &SimonStalenhagProvider::metaDataRequestFinished);
}

void SimonStalenhagProvider::metaDataRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    auto json = QJsonDocument::fromJson(job->data());
    auto imageObj = randomArrayValueByKey(json.object(), QLatin1String("simonstalenhag.se"));
    auto urlStr = imageObj.toObject().value(QLatin1String("imagebig")).toString();
    if (urlStr.isEmpty()) {
        Q_EMIT error(this);
        return;
    }
    QUrl picUrl(urlStr);
    KIO::StoredTransferJob *imageJob = KIO::storedGet(picUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &SimonStalenhagProvider::imageRequestFinished);
}

void SimonStalenhagProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }
    QByteArray data = job->data();
    mImage = QImage::fromData(data);
    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(SimonStalenhagProvider, "simonstalenhagprovider.json")

#include "simonstalenhagprovider.moc"
