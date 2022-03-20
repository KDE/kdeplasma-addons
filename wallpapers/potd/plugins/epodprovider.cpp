/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "epodprovider.h"

#include <QDebug>
#include <QRegExp>

#include <KIO/Job>
#include <KPluginFactory>

EpodProvider::EpodProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    const QUrl url(QStringLiteral("https://epod.usra.edu/blog/"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &EpodProvider::pageRequestFinished);
}

EpodProvider::~EpodProvider() = default;

void EpodProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data());

    const QString pattern = QStringLiteral("://epod.usra.edu/.a/*-pi");
    QRegExp exp(pattern);
    exp.setPatternSyntax(QRegExp::Wildcard);

    int pos = exp.indexIn(data) + pattern.length();
    const QString sub = data.mid(pos - 4, pattern.length() + 10);
    const QUrl url(QStringLiteral("https://epod.usra.edu/.a/%1-pi").arg(sub));
    KIO::StoredTransferJob *imageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &EpodProvider::imageRequestFinished);
}

void EpodProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    // FIXME: this really should be done in a thread as this can block
    potdProviderData()->wallpaperImage = QImage::fromData(job->data());
    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(EpodProvider, "epodprovider.json")

#include "epodprovider.moc"
