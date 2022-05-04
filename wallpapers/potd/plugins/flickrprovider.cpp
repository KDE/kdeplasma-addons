// SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
// SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
// SPDX-FileCopyrightText: 2008 Georges Toth <gtoth@trypill.org>
// SPDX-FileCopyrightText: 2021 Guo Yunhe <i@guoyunhe.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "flickrprovider.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include <QUrlQuery>

#include <KPluginFactory>

static QUrl buildUrl(const QDate &date, const QString apiKey)
{
    QUrl url(QLatin1String("https://api.flickr.com/services/rest/"));
    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("api_key"), apiKey);
    urlQuery.addQueryItem(QStringLiteral("method"), QStringLiteral("flickr.interestingness.getList"));
    urlQuery.addQueryItem(QStringLiteral("date"), date.toString(Qt::ISODate));
    // url_o might be either too small or too large.
    urlQuery.addQueryItem(QStringLiteral("extras"), QStringLiteral("url_k,url_h,url_o"));
    url.setQuery(urlQuery);

    return url;
}

FlickrProvider::FlickrProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    connect(this, &PotdProvider::configLoaded, this, &FlickrProvider::sendXmlRequest);

    loadConfig();
}

void FlickrProvider::sendXmlRequest(const QString &apiKey, const QString &apiSecret)
{
    Q_UNUSED(apiSecret);
    if (apiKey.isNull()) {
        refreshConfig();
        return;
    }

    mApiKey = apiKey;
    mActualDate = date().addDays(-2);

    const QUrl xmlUrl = buildUrl(mActualDate, apiKey);

    KIO::StoredTransferJob *xmlJob = KIO::storedGet(xmlUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(xmlJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::xmlRequestFinished);
}

void FlickrProvider::xmlRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        qDebug() << "xmlRequestFinished error";
        refreshConfig();
        return;
    }

    const QString data = QString::fromUtf8(job->data());

    // Clear the list
    m_photoList.clear();
    m_photoList.reserve(100);

    xml.clear();
    xml.addData(data);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            auto attributes = xml.attributes();
            if (xml.name() == QLatin1String("rsp")) {
                /* no pictures available for the specified parameters */
                if (attributes.value(QLatin1String("stat")).toString() != QLatin1String("ok")) {
                    Q_EMIT error(this);
                    qDebug() << "xmlRequestFinished error: no photos for the query";
                    return;
                }
            } else if (xml.name() == QLatin1String("photo")) {
                if (attributes.value(QLatin1String("ispublic")).toString() != QLatin1String("1")) {
                    continue;
                }

                const char *fallbackList[] = {"url_k", "url_h"};

                bool found = false;
                for (auto urlAttr : fallbackList) {
                    // Get the best url.
                    QLatin1String urlAttrString(urlAttr);
                    if (attributes.hasAttribute(urlAttrString)) {
                        QString title, userId, photoId;
                        if (attributes.hasAttribute(QStringLiteral("title"))) {
                            title = QTextDocumentFragment::fromHtml(attributes.value(QStringLiteral("title")).toString().trimmed()).toPlainText();
                        }
                        if (attributes.hasAttribute(QStringLiteral("owner")) && attributes.hasAttribute(QStringLiteral("id"))) {
                            userId = attributes.value(QStringLiteral("owner")).toString();
                            photoId = attributes.value(QStringLiteral("id")).toString();
                        }
                        m_photoList.emplace_back(PhotoEntry{
                            attributes.value(urlAttrString).toString(),
                            title,
                            userId,
                            photoId,
                        });
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
                        m_photoList.back().urlString = attributes.value(QLatin1String(originAttr)).toString();
                    }
                }
            }
        }
    }

    if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }

    if (m_photoList.begin() != m_photoList.end()) {
        const auto randomNumber = QRandomGenerator::global()->bounded(static_cast<int>(m_photoList.size()));
        const PhotoEntry &randomPhotoEntry = m_photoList.at(randomNumber);
        potdProviderData()->wallpaperRemoteUrl = QUrl(randomPhotoEntry.urlString);
        potdProviderData()->wallpaperTitle = randomPhotoEntry.title;

        /**
         * Visit the photo page to get the author
         * API document: https://www.flickr.com/services/api/misc.urls.html
         * https://www.flickr.com/photos/{user-id}/{photo-id}
         */
        if (!(randomPhotoEntry.userId.isEmpty() || randomPhotoEntry.photoId.isEmpty())) {
            potdProviderData()->wallpaperInfoUrl =
                QUrl(QStringLiteral("https://www.flickr.com/photos/%1/%2").arg(randomPhotoEntry.userId, randomPhotoEntry.photoId));
        }

        KIO::StoredTransferJob *imageJob = KIO::storedGet(potdProviderData()->wallpaperRemoteUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::imageRequestFinished);
    } else {
        qDebug() << "empty list";
    }
}

void FlickrProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    // Visit the photo page to get the author
    if (!potdProviderData()->wallpaperInfoUrl.isEmpty()) {
        KIO::StoredTransferJob *pageJob = KIO::storedGet(potdProviderData()->wallpaperInfoUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(pageJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::pageRequestFinished);
    }

    potdProviderData()->wallpaperImage = QImage::fromData(job->data());
}

void FlickrProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT finished(this); // No author is fine
        return;
    }

    const QString data = QString::fromUtf8(job->data()).simplified();

    // Example: <a href="/photos/jellybeanzgallery/" class="owner-name truncate" title="Go to Hammerchewer&#x27;s photostream"
    // data-track="attributionNameClick">Hammerchewer</a>
    QRegularExpression authorRegEx(QStringLiteral("<a.*?class=\"owner-name truncate\".*?>(.+?)</a>"));
    QRegularExpressionMatch match = authorRegEx.match(data);

    if (match.hasMatch()) {
        potdProviderData()->wallpaperAuthor = QTextDocumentFragment::fromHtml(match.captured(1).trimmed()).toPlainText();
    }

    Q_EMIT finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(FlickrProvider, "flickrprovider.json")

#include "flickrprovider.moc"
