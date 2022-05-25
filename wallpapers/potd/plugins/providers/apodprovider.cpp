/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "apodprovider.h"

#include <QDebug>
#include <QRegularExpression>
#include <QTextDocumentFragment>

#include <KIO/Job>
#include <KPluginFactory>

ApodProvider::ApodProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    potdProviderData()->wallpaperInfoUrl = QUrl(QStringLiteral("http://antwrp.gsfc.nasa.gov/apod/"));

    KIO::StoredTransferJob *job = KIO::storedGet(potdProviderData()->wallpaperInfoUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &ApodProvider::pageRequestFinished);
}

void ApodProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data()).simplified(); // Join lines so title match can work

    const QString pattern = QStringLiteral("<a href=\"(image/.*?)\"");
    QRegularExpression exp(pattern);
    const auto expMatch = exp.match(data);

    if (expMatch.hasMatch()) {
        const QString sub = expMatch.captured(1);
        potdProviderData()->wallpaperRemoteUrl = QUrl(QStringLiteral("http://antwrp.gsfc.nasa.gov/apod/") + sub);

        /**
         * Match title and author
         * Example:
         * <b> The Full Moon and the Dancer </b> <br>
         *
         * <b>Image Credit &
         * <a href="lib/about_apod.html#srapply">Copyright</a>:</b>
         *
         * <a href="https://www.instagram.com/through_my_lens_84/">Elena Pinna</a>
         */
        const QRegularExpression infoRegEx(QStringLiteral("<center>.*?<b>(.+?)</b>.*?Credit.*?:.*?</b>(.+?)</center>"));
        const QRegularExpressionMatch match = infoRegEx.match(data);

        if (match.hasMatch()) {
            potdProviderData()->wallpaperTitle = QTextDocumentFragment::fromHtml(match.captured(1).trimmed()).toPlainText();
            potdProviderData()->wallpaperAuthor = QTextDocumentFragment::fromHtml(match.captured(2).trimmed()).toPlainText();
        }

        KIO::StoredTransferJob *imageJob = KIO::storedGet(potdProviderData()->wallpaperRemoteUrl, KIO::NoReload, KIO::HideProgressInfo);
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

    potdProviderData()->wallpaperImage = QImage::fromData(job->data());
    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(ApodProvider, "apodprovider.json")

#include "apodprovider.moc"
