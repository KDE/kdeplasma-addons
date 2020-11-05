/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   SPDX-FileCopyrightText: 2008 Georges Toth <gtoth@trypill.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "flickrprovider.h"

#include <QUrlQuery>
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <KPluginFactory>
#include <KIO/Job>

static QUrl buildUrl(const QDate &date, const QString apiKey)
{
    QUrl url(QLatin1String( "https://api.flickr.com/services/rest/"));
    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("api_key"), apiKey);
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
    const QUrl keyUrl(QStringLiteral("https://invent.kde.org/plasma/kdeplasma-addons/-/raw/master/dataengines/potd/flickrprovider.conf"));

    KIO::StoredTransferJob *keyJob = KIO::storedGet(keyUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(keyJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::keyRequestFinished);
}

void FlickrProvider::keyRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *keyJob = static_cast<KIO::StoredTransferJob *>( _job );
    if (keyJob->error()) {
        emit error(this);
        qDebug() << "keyRequestFinished error: failed to fetch data";
        return;
    }

    const QString keyData = QString::fromUtf8( keyJob->data() );

    QRegularExpression re( QStringLiteral("API_KEY=(.*)" ) );
    QRegularExpressionMatch match = re.match(keyData);
    if ( !match.hasMatch() ) {
        emit error(this);
        qDebug() << "keyRequestFinished error: failed to parse data";
        return;
    }
    mApiKey = match.captured(1);

    mActualDate = date();

    const QUrl xmlUrl = buildUrl(mActualDate, mApiKey);

    KIO::StoredTransferJob *xmlJob = KIO::storedGet(xmlUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(xmlJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::xmlRequestFinished);
}

FlickrProvider::~FlickrProvider() = default;

QImage FlickrProvider::image() const
{
    return mImage;
}

void FlickrProvider::xmlRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if (job->error()) {
        emit error(this);
        qDebug() << "xmlRequestFinished error";
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
                        QUrl url = buildUrl(mActualDate, mApiKey);
                        KIO::StoredTransferJob *pageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
                        connect(pageJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::xmlRequestFinished);
                        mFailureNumber++;
                        return;
                    } else {
                        emit error(this);
                        qDebug() << "xmlRequestFinished error";
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
