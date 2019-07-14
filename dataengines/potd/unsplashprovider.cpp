/*
 *   Copyright 2019 Guo Yunhe <i@guoyunhe.me>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "unsplashprovider.h"

#include <QDebug>
#include <QRegularExpression>

#include <KPluginFactory>
#include <KIO/Job>

UnsplashProvider::UnsplashProvider(QObject* parent, const QVariantList& args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://unsplash.com/wallpaper/1065396/desktop-wallpapers"));

    KIO::StoredTransferJob* job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &UnsplashProvider::pageRequestFinished);
}

UnsplashProvider::~UnsplashProvider() = default;

QImage UnsplashProvider::image() const
{
    return mImage;
}

void UnsplashProvider::pageRequestFinished(KJob* _job)
{
    KIO::StoredTransferJob* job = static_cast<KIO::StoredTransferJob*>(_job);
    if (job->error()) {
        emit error(this);
        return;
    }

    const QString html = QString::fromUtf8(job->data());

    // "?ixlib" will filter out the banner image which rarely change...
    QRegularExpression re(QStringLiteral("src=\"(https://images\\.unsplash\\.com/photo-\\w+-\\w+)\\?ixlib"));

    QRegularExpressionMatchIterator i = re.globalMatch(html);

    QStringList urls;

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString url = match.captured(1);
        urls << url;
    }

    if (urls.size() > 0) {
        // Pick a ramdom photo because the wallpaper page doesn't update every day
        QUrl picUrl(urls.at(rand() % urls.size())); // url to full size photo (compressed)
        KIO::StoredTransferJob* imageJob = KIO::storedGet(picUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &UnsplashProvider::imageRequestFinished);
        return;
    } else {
        emit error(this);
        return;
    }
}

void UnsplashProvider::imageRequestFinished(KJob* _job)
{
    KIO::StoredTransferJob* job = static_cast<KIO::StoredTransferJob*>(_job);
    if (job->error()) {
        emit error(this);
        return;
    }
    QByteArray data = job->data();
    mImage = QImage::fromData(data);
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(UnsplashProvider, "unsplashprovider.json")

#include "unsplashprovider.moc"
