// SPDX-FileCopyrightText: 2020 Guo Yunhe <i@guoyunhe.me>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pixabayprovider.h"

#include <QUrlQuery>
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <KPluginFactory>
#include <KIO/Job>

static QUrl buildUrl(const QString key, const QString type, const QString orientation, const QString category)
{
    QUrl url(QLatin1String( "https://pixabay.com/api/"));
    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("key"), key);
    urlQuery.addQueryItem(QStringLiteral("image_type"), type);
    urlQuery.addQueryItem(QStringLiteral("orientation"), orientation);
    urlQuery.addQueryItem(QStringLiteral("category"), category);
    urlQuery.addQueryItem(QStringLiteral("editors_choice"), QStringLiteral("true"));
    urlQuery.addQueryItem(QStringLiteral("safesearch"), QStringLiteral("true"));
    urlQuery.addQueryItem(QStringLiteral("order"), QStringLiteral("latest"));
    url.setQuery(urlQuery);

    return url;
}

PixabayProvider::PixabayProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    mImageType = args[1];
    mOrientation = args[2];
    mCategory = args[3];

    const QUrl keyUrl(QStringLiteral("https://invent.kde.org/plasma/kdeplasma-addons/-/raw/master/dataengines/potd/pixabayprovider.conf"));

    KIO::StoredTransferJob *keyJob = KIO::storedGet(keyUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(keyJob, &KIO::StoredTransferJob::finished, this, &PixabayProvider::keyRequestFinished);
}

void PixabayProvider::keyRequestFinished(KJob *_job)
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

    const QUrl jsonUrl = buildUrl(mApiKey, mImageType, mOrientation, mCategory);

    qDebug() << jsonUrl;
    return;

    KIO::StoredTransferJob *jsonJob = KIO::storedGet(jsonUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(jsonJob, &KIO::StoredTransferJob::finished, this, &PixabayProvider::jsonRequestFinished);
}

PixabayProvider::~PixabayProvider() = default;

QImage PixabayProvider::image() const
{
    return mImage;
}

void PixabayProvider::jsonRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if (job->error()) {
        emit error(this);
        qDebug() << "jsonRequestFinished error";
        return;
    }

    const QString data = QString::fromUtf8( job->data() );

    // Clear the list
    m_photoList.clear();

    json.clear();
    json.addData(data);

    while (!json.atEnd()) {
        json.readNext();

        if (json.isStartElement()) {
            auto attributes = json.attributes();
            if (json.name() == QLatin1String("rsp")) {
                const int maxFailure = 5;
                /* no pictures available for the specified parameters */
                if (attributes.value ( QLatin1String( "stat" ) ).toString() != QLatin1String( "ok" )) {
                    if (mFailureNumber < maxFailure) {
                        /* To be sure, decrement the date to two days earlier... @TODO */
                        mActualDate = mActualDate.addDays(-2);
                        QUrl url = buildUrl(mActualDate, mApiKey);
                        KIO::StoredTransferJob *pageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
                        connect(pageJob, &KIO::StoredTransferJob::finished, this, &PixabayProvider::jsonRequestFinished);
                        mFailureNumber++;
                        return;
                    } else {
                        emit error(this);
                        qDebug() << "jsonRequestFinished error";
                        return;
                    }
                }
            } else if (json.name() == QLatin1String( "photo" )) {
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

    if (json.error() && json.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qWarning() << "XML ERROR:" << json.lineNumber() << ": " << json.errorString();
    }

    if (m_photoList.begin() != m_photoList.end()) {
        QUrl url( m_photoList.at(QRandomGenerator::global()->bounded(m_photoList.size())) );
            KIO::StoredTransferJob *imageJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
            connect(imageJob, &KIO::StoredTransferJob::finished, this, &PixabayProvider::imageRequestFinished);
    } else {
        qDebug() << "empty list";
    }
}

void PixabayProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
        emit error(this);
        return;
    }

    mImage = QImage::fromData( job->data() );
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(PixabayProvider, "pixabayprovider.json")

#include "pixabayprovider.moc"
