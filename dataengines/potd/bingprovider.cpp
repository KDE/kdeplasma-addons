/*
 *   Copyright 2017 Weng Xuetian <wengxt@gmail.com>
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

#include "bingprovider.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

#include <KPluginFactory>
#include <KIO/Job>

BingProvider::BingProvider(QObject* parent, const QVariantList& args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1"));

    KIO::StoredTransferJob* job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &BingProvider::pageRequestFinished);
}

BingProvider::~BingProvider() = default;

QImage BingProvider::image() const
{
    return mImage;
}

void BingProvider::pageRequestFinished(KJob* _job)
{
    KIO::StoredTransferJob* job = static_cast<KIO::StoredTransferJob*>(_job);
    if (job->error()) {
        emit error(this);
        return;
    }

    auto json = QJsonDocument::fromJson(job->data());
    do {
        if (json.isNull()) {
            break;
        }
        auto imagesArray = json.object().value(QLatin1String("images"));
        if (!imagesArray.isArray() || imagesArray.toArray().size() <= 0) {
            break;
        }
        auto imageObj = imagesArray.toArray().at(0);
        if (!imageObj.isObject()) {
            break;
        }
        auto url = imageObj.toObject().value(QLatin1String("url"));
        if (!url.isString() || url.toString().isEmpty()) {
            break;
        }
        QUrl picUrl(QStringLiteral("https://www.bing.com/%1").arg(url.toString()));
        KIO::StoredTransferJob* imageJob = KIO::storedGet(picUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &BingProvider::imageRequestFinished);
        return;
    } while (0);

    emit error(this);
    return;
}

void BingProvider::imageRequestFinished(KJob* _job)
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

K_PLUGIN_CLASS_WITH_JSON(BingProvider, "bingprovider.json")

#include "bingprovider.moc"
