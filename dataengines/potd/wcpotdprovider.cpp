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

#include <kio/job.h>

class WcpotdProvider::Private
{
  public:
    Private( WcpotdProvider *parent )
      : mParent( parent )
    {
    }

    void pageRequestFinished( KJob* );
    void imageRequestFinished( KJob* );
    void parsePage();

    WcpotdProvider *mParent;
    QImage mImage;
};

void WcpotdProvider::Private::pageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
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
            mParent->connect( imageJob, SIGNAL(finished(KJob*)), SLOT(imageRequestFinished(KJob*)) );
            return;
        }
    }

    emit mParent->error(mParent);
}

void WcpotdProvider::Private::imageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }
    QByteArray data = job->data();
    mImage = QImage::fromData( data );
    emit mParent->finished( mParent );
}

WcpotdProvider::WcpotdProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    QUrl url(QStringLiteral("https://commons.wikimedia.org/w/api.php"));

    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("action"), QStringLiteral("parse"));
    urlQuery.addQueryItem(QStringLiteral("text"), QStringLiteral("{{Potd}}"));
    urlQuery.addQueryItem(QStringLiteral("contentmodel"), QStringLiteral("wikitext"));
    urlQuery.addQueryItem(QStringLiteral("prop"), QStringLiteral("images"));
    urlQuery.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    url.setQuery(urlQuery);

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    connect( job, SIGNAL(finished(KJob*)), SLOT(pageRequestFinished(KJob*)) );
}

WcpotdProvider::~WcpotdProvider()
{
    delete d;
}

QImage WcpotdProvider::image() const
{
    return d->mImage;
}

K_PLUGIN_FACTORY_WITH_JSON(WcpotdProviderFactory, "wcpotdprovider.json", registerPlugin<WcpotdProvider>();)

#include "wcpotdprovider.moc"
#include "moc_wcpotdprovider.cpp"
