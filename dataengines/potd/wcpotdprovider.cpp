/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>
 *   Copyright 2016 Weng Xuetian <wengxt@gmail.com>
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

#include "wcpotdprovider.h"

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QImage>
#include <QDebug>

#include <KPluginFactory>
#include <KIO/Job>


WcpotdProvider::WcpotdProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    QUrl url(QStringLiteral("https://commons.wikimedia.org/w/api.php"));

    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("action"), QStringLiteral("parse"));
    urlQuery.addQueryItem(QStringLiteral("text"), QStringLiteral("{{Potd}}"));
    urlQuery.addQueryItem(QStringLiteral("contentmodel"), QStringLiteral("wikitext"));
    urlQuery.addQueryItem(QStringLiteral("prop"), QStringLiteral("images"));
    urlQuery.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    url.setQuery(urlQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &WcpotdProvider::pageRequestFinished);
}

WcpotdProvider::~WcpotdProvider() = default;

QImage WcpotdProvider::image() const
{
    return mImage;
}

void WcpotdProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit error(this);
	return;
    }

    auto jsonImageArray = QJsonDocument::fromJson(job->data())
        .object().value(QLatin1String("parse"))
        .toObject().value(QLatin1String("images"))
        .toArray();

    if (jsonImageArray.size() > 0) {
        const QString imageFile = jsonImageArray.at(0).toString();
        if (!imageFile.isEmpty()) {
            const QUrl picUrl(QLatin1String("https://commons.wikimedia.org/wiki/Special:FilePath/") + imageFile);
            KIO::StoredTransferJob *imageJob = KIO::storedGet( picUrl, KIO::NoReload, KIO::HideProgressInfo );
            connect(imageJob, &KIO::StoredTransferJob::finished, this, &WcpotdProvider::imageRequestFinished);
            return;
        }
    }

    emit error(this);
}

void WcpotdProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit error(this);
	return;
    }
    QByteArray data = job->data();
    mImage = QImage::fromData( data );
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(WcpotdProvider, "wcpotdprovider.json")

#include "wcpotdprovider.moc"
