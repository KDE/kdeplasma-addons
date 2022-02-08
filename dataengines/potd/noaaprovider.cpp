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
    const QUrl url(QStringLiteral("https://www.nesdis.noaa.gov/real-time-imagery/imagery-collections/image-of-the-day"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &NOAAProvider::listPageRequestFinished);
}

NOAAProvider::~NOAAProvider() = default;

void NOAAProvider::listPageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data()).simplified();

    // Using regular expression could be fragile in such case, but the HTML
    // NOAA page itself is not a valid XML file and unfortunately it could
    // not be parsed successfully till the content we want. And we do not want
    // to use heavy weight QtWebkit. So we use QRegularExpression to capture
    // the wanted url here.
    // Example: <a href="/news/hunga-tonga-hunga-haapai-erupts-again" hreflang="en">Hunga Tonga-Hunga Ha&#039;apai Erupts Again</a>
    QUrl url;
    const QRegularExpression re("<div class=\"item-list\">.*?<li>.*?<a href=\"(.+?)\".*?>");
    auto result = re.match(data);
    if (result.hasMatch()) {
        url = QUrl(QStringLiteral("https://www.nesdis.noaa.gov") + result.captured(1));
    }
    if (!url.isValid()) {
        Q_EMIT error(this);
        return;
    }

    KIO::StoredTransferJob *pageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(pageJob, &KIO::StoredTransferJob::finished, this, &NOAAProvider::pageRequestFinished);
}

void NOAAProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data()).simplified();

    /**
     * Example:
     * <a class="call-to-action blue" href="/s3dl?path=/s3/2022-01/20220113-HungaVolcanoA.gif">
     *      <span class="fas fa-download"></span> Download Animation
     * </a>
     */
    const QRegularExpression re("<a class=\"call-to-action.*?\" href=\"(.+?)\">.*?Download.*?</a>");
    const QRegularExpressionMatch result = re.match(data);
    QUrl url;
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
        qWarning() << "Failed to get the latest image from NOAAProvider!";
        Q_EMIT error(this);
        return;
    }

    potdProviderData()->wallpaperImage = QImage::fromData(job->data());
    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(NOAAProvider, "noaaprovider.json")

#include "noaaprovider.moc"
