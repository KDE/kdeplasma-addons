/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "apodprovider.h"

#include <QDebug>
#include <QRegExp>

#include <KIO/Job>
#include <KPluginFactory>

ApodProvider::ApodProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("http://antwrp.gsfc.nasa.gov/apod/"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &ApodProvider::pageRequestFinished);
}

ApodProvider::~ApodProvider() = default;

QImage ApodProvider::image() const
{
    return mImage;
}

void ApodProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        emit error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data());

    const QString pattern = QStringLiteral("<a href=\"(image/.*)\"");
    QRegExp exp(pattern);
    exp.setMinimal(true);
    if (exp.indexIn(data) != -1) {
        const QString sub = exp.cap(1);
        const QUrl url(QLatin1String("http://antwrp.gsfc.nasa.gov/apod/") + sub);
        KIO::StoredTransferJob *imageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &ApodProvider::imageRequestFinished);
    } else {
        emit error(this);
    }
}

void ApodProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        emit error(this);
        return;
    }

    mImage = QImage::fromData(job->data());
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(ApodProvider, "apodprovider.json")

#include "apodprovider.moc"
