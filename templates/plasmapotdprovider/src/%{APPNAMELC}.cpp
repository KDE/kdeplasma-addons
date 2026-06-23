/*
 *   SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "%{APPNAMELC}.h"

// KF
#include <KPluginFactory>
#include <KIO/Job>
#include <KIO/StoredTransferJob>


%{APPNAME}::%{APPNAME}(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : PotdProvider(parent, metaData, args)
{
    // TODO: replace with url to data about what the current picture of the day is
    const QUrl potdFeed(QStringLiteral("https://kde.org"));

    KIO::StoredTransferJob* job = KIO::storedGet(potdFeed, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished,
            this, &%{APPNAME}::handleFinishedFeedRequest);
}

void %{APPNAME}::handleFinishedFeedRequest(KJob *job)
{
    KIO::StoredTransferJob *requestJob = static_cast<KIO::StoredTransferJob*>(job);
    if (requestJob->error()) {
        Q_EMIT error(this);
        return;
    }

    // TODO: read url to image from requestJob->data()
    const QUrl picureUrl(QStringLiteral("https://planet.kde.org/img/planet.svg"));

    KIO::StoredTransferJob *imageJob = KIO::storedGet(picureUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished,
            this, &%{APPNAME}::handleFinishedImageRequest);
}

void %{APPNAME}::handleFinishedImageRequest(KJob *job)
{
    KIO::StoredTransferJob *requestJob = static_cast<KIO::StoredTransferJob*>(job);
    if (requestJob->error()) {
        Q_EMIT error(this);
        return;
    }

    const auto image = QImage::fromData(requestJob->data());

    if (image.isNull()) {
        Q_EMIT error(this);
        return;
    }

    Q_EMIT finished(this, image);
}


K_PLUGIN_CLASS_WITH_JSON(%{APPNAME}, "%{APPNAMELC}.json")

#include "%{APPNAMELC}.moc"
