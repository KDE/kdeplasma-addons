/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "noaaprovider.h"

#include <QDebug>
#include <QRegularExpression>

#include <KIO/Job>
#include <KPluginFactory>

NOAAProvider::NOAAProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://www.nesdis.noaa.gov/content/imagery-and-data"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &NOAAProvider::pageRequestFinished);
}

NOAAProvider::~NOAAProvider() = default;

QImage NOAAProvider::image() const
{
    return mImage;
}

void NOAAProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data());

    // Using regular expression could be fragile in such case, but the HTML
    // NOAA page itself is not a valid XML file and unfortunately it could
    // not be parsed successfully till the content we want. And we do not want
    // to use heavy weight QtWebkit. So we use QRegularExpression to capture
    // the wanted url here.
    QUrl url;
    QRegularExpression re(QStringLiteral("\"(/sites/default/files/.*\\.jpg)\""));
    auto result = re.match(data);
    if (result.hasMatch()) {
        url = QUrl(QStringLiteral("https://www.nesdis.noaa.gov") + result.captured(1));
    }
    if (!url.isValid()) {
        Q_EMIT error(this);
        return;
    }

    KIO::StoredTransferJob *imageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &NOAAProvider::imageRequestFinished);
}

void NOAAProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    mImage = QImage::fromData(job->data());
    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(NOAAProvider, "noaaprovider.json")

#include "noaaprovider.moc"
