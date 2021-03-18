/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   SPDX-FileCopyrightText: 2013 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "natgeoprovider.h"

#include <QDebug>

#include <KIO/Job>
#include <KPluginFactory>

NatGeoProvider::NatGeoProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://www.nationalgeographic.com/photography/photo-of-the-day/"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &NatGeoProvider::pageRequestFinished);
}

NatGeoProvider::~NatGeoProvider() = default;

QImage NatGeoProvider::image() const
{
    return mImage;
}

void NatGeoProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data());
    const QStringList lines = data.split(QLatin1Char('\n'));

    QString url;

    re.setPattern(QStringLiteral("<meta.+property=\"og:image\"\\s+content=\"(.*?)\".*/>"));

    for (int i = 0; i < lines.size(); i++) {
        QRegularExpressionMatch match = re.match(lines.at(i));
        if (match.hasMatch()) {
            url = match.captured(1);
        }
    }

    if (url.isEmpty()) {
        Q_EMIT error(this);
        return;
    }

    KIO::StoredTransferJob *imageJob = KIO::storedGet(QUrl(url), KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &NatGeoProvider::imageRequestFinished);
}

void NatGeoProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    mImage = QImage::fromData(job->data());
    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(NatGeoProvider, "natgeoprovider.json")

#include "natgeoprovider.moc"
