/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>
 *   Copyright  2008 by Georges Toth <gtoth@trypill.org>
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

#include "flickrprovider.h"

#include <QUrlQuery>
#include <QDebug>
#include <QRandomGenerator>
#include <KPluginFactory>
#include <KIO/Job>

#define FLICKR_API_KEY QStringLiteral("11829a470557ad8e10b02e80afacb3af")

static
QUrl buildUrl(const QDate &date)
{
    QUrl url(QLatin1String( "https://api.flickr.com/services/rest/"));
    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("api_key"), FLICKR_API_KEY);
    urlQuery.addQueryItem(QStringLiteral("method"), QStringLiteral("flickr.interestingness.getList"));
    urlQuery.addQueryItem(QStringLiteral("date"), date.toString(Qt::ISODate));
    // url_o might be either too small or too large.
    urlQuery.addQueryItem(QStringLiteral("extras"), QStringLiteral("url_k,url_h,url_o"));
    url.setQuery(urlQuery);

    return url;
}

FlickrProvider::FlickrProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    mActualDate = date();

    const QUrl url = buildUrl(mActualDate);

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &FlickrProvider::pageRequestFinished);
}

FlickrProvider::~FlickrProvider() = default;

QImage FlickrProvider::image() const
{
    return mImage;
}

void FlickrProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if (job->error()) {
        emit error(this);
        qDebug() << "pageRequestFinished error";
        return;
    }

    const QString data = QString::fromUtf8( job->data() );

    // Clear the list
    m_photoList.clear();

    xml.clear();
    xml.addData(data);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            auto attributes = xml.attributes();
            if (xml.name() == QLatin1String("rsp")) {
                const int maxFailure = 5;
                /* no pictures available for the specified parameters */
                if (attributes.value ( QLatin1String( "stat" ) ).toString() != QLatin1String( "ok" )) {
                    if (mFailureNumber < maxFailure) {
                        /* To be sure, decrement the date to two days earlier... @TODO */
                        mActualDate = mActualDate.addDays(-2);
                        QUrl url = buildUrl(mActualDate);
                        KIO::StoredTransferJob *pageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
                        connect(pageJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::pageRequestFinished);
                        mFailureNumber++;
                        return;
                    } else {
                        emit error(this);
                        qDebug() << "pageRequestFinished error";
                        return;
                    }
                }
            } else if (xml.name() == QLatin1String( "photo" )) {
                if (attributes.value ( QLatin1String( "ispublic" ) ).toString() != QLatin1String( "1" )) {
                    continue;
                }

                const char *fallbackList[] = {
                    "url_k", "url_h"
                };

                bool found = false;
                for (auto urlAttr : fallbackList) {
                    // Get the best url.
                    QLatin1String urlAttrString(urlAttr);
                    if (attributes.hasAttribute(urlAttrString)) {
                        m_photoList.append(attributes.value(urlAttrString).toString());
                        found = true;
                        break;
                    }
                }

                // The logic here is, if url_h or url_k are present, url_o must
                // has higher quality, otherwise, url_o is worse than k/h size.
                // If url_o is better, prefer url_o.
                if (found) {
                    QLatin1String originAttr("url_o");
                    if (attributes.hasAttribute(originAttr)) {
                        m_photoList.back() = attributes.value(QLatin1String(originAttr)).toString();
                    }
                }
            }
        }
    }

    if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }

    if (m_photoList.begin() != m_photoList.end()) {
        QUrl url( m_photoList.at(QRandomGenerator::global()->bounded(m_photoList.size())) );
            KIO::StoredTransferJob *imageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
            connect(imageJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::imageRequestFinished);
    } else {
        qDebug() << "empty list";
    }
}

void FlickrProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
        emit error(this);
        return;
    }

    mImage = QImage::fromData( job->data() );
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(FlickrProvider, "flickrprovider.json")

#include "flickrprovider.moc"
