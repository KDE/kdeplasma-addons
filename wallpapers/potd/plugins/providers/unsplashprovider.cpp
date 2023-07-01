/*
 *   SPDX-FileCopyrightText: 2019 Guo Yunhe <i@guoyunhe.me>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "unsplashprovider.h"

#include <QRegularExpression>

#include <KIO/Job>
#include <KPluginFactory>

UnsplashProvider::UnsplashProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    QString collectionId = QStringLiteral("1065976");
    if (!args.empty()) {
        const QRegularExpression re(QStringLiteral("^\\d+$"));
        const QString arg = args[0].toString();
        if (re.match(arg).hasMatch()) {
            collectionId = arg;
        }
    }
    const QUrl url(QStringLiteral("https://source.unsplash.com/collection/%1/3840x2160/daily").arg(collectionId));

    potdProviderData()->wallpaperInfoUrl = QUrl(QStringLiteral("https://unsplash.com/collections/%1").arg(collectionId));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &UnsplashProvider::imageRequestFinished);
}

void UnsplashProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }
    QByteArray data = job->data();
    potdProviderData()->wallpaperImage = QImage::fromData(data);
    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(UnsplashProvider, "unsplashprovider.json")

#include "unsplashprovider.moc"
