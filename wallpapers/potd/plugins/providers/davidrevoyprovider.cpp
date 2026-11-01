/*
 *   SPDX-FileCopyrightText: 2026 Guillermo Villafuerte <me@strongville.dev>
 *
 *   SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 */

#include "davidrevoyprovider.h"

#include <random>
#include <zlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>

#include <KIO/StoredTransferJob>
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

    // Plasma 6.6 LTS release date
    std::mt19937 randomEngine(QDate(2026, 2, 17).daysTo(QDate::currentDate()) * 50 + key.size());
    std::uniform_int_distribution<int> distrib(0, array.size() - 1);

    return array.at(distrib(randomEngine));
}

DavidRevoyProvider::DavidRevoyProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    m_author = QStringLiteral("David Revoy");

    const QUrl url(QStringLiteral("https://autoconfig.kde.org/potd/davidrevoy-data.json"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &DavidRevoyProvider::metaDataRequestFinished);
}

void DavidRevoyProvider::metaDataRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    auto json = QJsonDocument::fromJson(job->data());
    auto defaultUrl = json.object().value(QStringLiteral("defaultUrl")).toString();
    auto imageObj = randomArrayValueByKey(json.object(), QLatin1String("imageList")).toObject();
    auto urlStr = imageObj.value(QStringLiteral("image")).toString();
    if (urlStr.isEmpty()) {
        Q_EMIT error(this);
        return;
    }
    m_remoteUrl = QUrl(urlStr);

    auto url = imageObj.value(QStringLiteral("url")).toString();
    if (!url.isEmpty()) {
        m_infoUrl = QUrl(url);
    } else {
        m_infoUrl = QUrl(defaultUrl);
    }

    const QString titleStr = imageObj.value(QStringLiteral("name")).toString();
    if (!titleStr.isEmpty()) {
        m_title = titleStr;
    }

    const QString checksumStr = imageObj.value(QStringLiteral("checksum")).toString();
    if (!checksumStr.isEmpty()) {
        m_checksum = checksumStr;
    }

    KIO::StoredTransferJob *imageJob = KIO::storedGet(m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &DavidRevoyProvider::imageRequestFinished);
}

void DavidRevoyProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }
    m_imageData = job->data();

    if (m_checksum.isEmpty()) {
        Q_EMIT finished(this, QImage::fromData(m_imageData));
        return;
    }

    auto *checksumJob = new ChecksumJob(m_imageData, this);
    connect(checksumJob, &KJob::result, this, &DavidRevoyProvider::checksumCalculationFinished);
    checksumJob->start();
}

void DavidRevoyProvider::checksumCalculationFinished(KJob *job)
{
    auto *checksumJob = static_cast<ChecksumJob *>(job);
    auto calculated_checksum = QString::asprintf("%08x", checksumJob->checksum());

    if (m_checksum != calculated_checksum) {
        Q_EMIT error(this);
        return;
    }

    Q_EMIT finished(this, QImage::fromData(m_imageData));
}

ChecksumJob::ChecksumJob(const QByteArray &data, QObject *parent)
    : KJob(parent)
    , m_data(data)
{
}

quint32 ChecksumJob::checksum() const
{
    return m_checksum;
}

void ChecksumJob::start()
{
    QTimer::singleShot(0, this, &ChecksumJob::calculateChecksum);
}

void ChecksumJob::calculateChecksum()
{
    uLong crc = crc32(0L, Z_NULL, 0);

    const auto *data = reinterpret_cast<const Bytef *>(m_data.constData());
    qsizetype remaining = m_data.size();
    qsizetype offset = 0;

    while (remaining > 0) {
        const uInt chunkSize = static_cast<uInt>(qMin(remaining, ChecksumChunkSize));
        crc = crc32(crc, data + offset, chunkSize);
        offset += chunkSize;
        remaining -= chunkSize;
    }

    m_checksum = static_cast<quint32>(crc);
    emitResult();
}

K_PLUGIN_CLASS_WITH_JSON(DavidRevoyProvider, "davidrevoyprovider.json")

#include "davidrevoyprovider.moc"
