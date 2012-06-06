/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009-2010 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright 2012 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

#include "timelinesource.h"
#include "timelineservice.h"
#include "tweetjob.h"

#include <QFile>
#include <QXmlStreamReader>

#include <qjson/parser.h>

#include <KDebug>
#include <KIO/Job>

#include "imagesource.h"
#include "koauth.h"

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

class TimelineSourcePrivate {

public:
    TimelineSourcePrivate()
    {
        newestId = 0;
        oldestId = 0;
    }
    qulonglong newestId;
    qulonglong oldestId;
    QString loadMoreUrl;
    QString refreshUrl;
};

TimelineSource::TimelineSource(const QString &serviceUrl, RequestType requestType, KOAuth::KOAuth *oauthHelper, const QStringList &parameters, QObject* parent)
    : Plasma::DataContainer(parent),
      m_serviceBaseUrl(serviceUrl),
      m_needsAuthorization(true),
      m_requestType(requestType),
      m_job(0),
      m_authHelper(oauthHelper),
      m_parameters(parameters),
      m_authJob(0),
      m_qcaInitializer(0)
{
    d = new TimelineSourcePrivate;
    setObjectName(QLatin1String("Timeline"));
    // set up the url
    QString query;

    m_params.clear();
//     if (m_parameters.count()) {
//         m_params.insert("q", m_parameters[0].toLocal8Bit());
//     }

    // default arguments
    m_params.insert("include_entities", "true");
    m_params.insert("include_rts", "true");
    m_params.insert("count", "50");
    m_params.insert("trim_user", "false");

    // parse URL from QML runtime, possibly overwrite defaults
    bool hasQuery = false;
    const QString &pa = m_parameters[0].toLocal8Bit();
    if (!pa.isEmpty()) {
        const QStringList &tokens = pa.split(QLatin1Char('&'));
        foreach (const QString &t, tokens) {
            const QStringList &pair = t.split(QLatin1Char('='));
            if (pair.count() == 2) {
                const QByteArray &n = QUrl::toPercentEncoding(pair.at(0).toLocal8Bit());
                const QByteArray &v = QUrl::toPercentEncoding(pair.at(1).toLocal8Bit());
//                 kDebug() << "       inserted: " << n << v;
                query.append(QString("%1=%2&").arg(QString(n),  QString(v)));
                m_params.insert(n, v);
                if (n == QByteArray("q") && !v.isEmpty()) {
                    hasQuery = true;
                }
                //inserted =
            } else {
                kWarning() << "Parsing problem expected 2 values, got: " << pa << pair;
            }
        }
    }
    switch (m_requestType) {
    case CustomTimeline:
    case SearchTimeline:
        if (!hasQuery) {
            return;
        }
        if (m_serviceBaseUrl.host().endsWith("twitter.com")) {
            m_url = KUrl("http://search.twitter.com/search.json");
        } else if (m_serviceBaseUrl.host().endsWith("identi.ca")) {
            m_url = KUrl("http://identi.ca/api/search.json");
        } else {

        }
        m_params.insert("show_user", "true");
        m_params.insert("rpp", "88");
        //m_params.insert("result_type", "mixed");
        m_needsAuthorization = false;
//         kDebug() << "Search or Custom Url: " << m_url << m_serviceBaseUrl;
        break;
   case Profile:
        m_url = KUrl(m_serviceBaseUrl, QString("users/show/%1.json").arg(parameters.at(0)));
        m_needsAuthorization = false;
        break;
    case DirectMessages:
        m_url = KUrl(m_serviceBaseUrl, "direct_messages.json");
        break;
    case Replies:
        m_url = KUrl(m_serviceBaseUrl, "statuses/mentions.json");
//         kDebug() << "XXXX " << m_url;
        break;
    case TimelineWithFriends:
        m_url = KUrl(m_serviceBaseUrl, "statuses/home_timeline.json");
        break;
    case Timeline:
    default:
        m_url = KUrl(m_serviceBaseUrl, "statuses/user_timeline.json");
        break;
    }
    //m_params.insert("count", QString("100").toLocal8Bit());

//     kDebug() << "authorized." << m_authHelper->isAuthorized();
    if (!m_needsAuthorization || m_authHelper->isAuthorized()) {
        update();
    }
    if (m_needsAuthorization) {
        connect(m_authHelper, SIGNAL(authorized()), SLOT(update()));
    }
}

TimelineSource::~TimelineSource()
{
}

bool TimelineSource::needsAuthorization() const
{
    return m_needsAuthorization;
}

Plasma::Service* TimelineSource::createService()
{
    return new TimelineService(this);
}

void TimelineSource::startAuthorization(const QString& user, const QString& password)
{
    emit authorize(m_serviceBaseUrl.pathOrUrl(), user, password);
}

void TimelineSource::forgetAccount(const QString& user, const QString& serviceUrl)
{
    m_authHelper->forgetAccount(user, serviceUrl);
    emit accountRemoved(user + "@" + serviceUrl);
}


void TimelineSource::setOAuthHelper(KOAuth::KOAuth* authHelper)
{
    m_authHelper = authHelper;
}

KOAuth::KOAuth* TimelineSource::oAuthHelper()
{
    return m_authHelper;
}

void TimelineSource::setPassword(const QString &password)
{
    m_authHelper->authorize(m_serviceBaseUrl.pathOrUrl(), m_user, password);
}

QString TimelineSource::password() const
{
    return m_url.pass();
}

QString TimelineSource::account() const
{
    return m_user;
}

KUrl TimelineSource::serviceBaseUrl() const
{
    return m_serviceBaseUrl;
}

QByteArray TimelineSource::oauthToken() const
{
    return m_oauthToken;
}

QByteArray TimelineSource::oauthTokenSecret() const
{
    return m_oauthTokenSecret;
}

KIO::Job* TimelineSource::loadMore()
{
    m_params.replace("max_id", QString::number(d->oldestId).toLocal8Bit());
    kDebug() << "Loading tweets before " << QString::number(d->oldestId);
    return update(true);
}

KIO::Job* TimelineSource::update(bool forcedUpdate)
{
//     kDebug() << "Refresh() ... ";
    if (!m_authHelper->isAuthorized()) {
        return 0;
    }
    if (m_job) {
        // We are already performing a fetch, let's not bother starting over
        return 0;
    }


    // Create a KIO job to get the data from the web service
    QByteArray ps;
    ps = m_authHelper->userParameters(m_params);

    KUrl u = KUrl(m_url.pathOrUrl() + ps);
//     kDebug() << "Creating job..." << u << " P: " << ps << " User: " << m_authHelper->user();
    m_job = KIO::get(u, KIO::Reload, KIO::HideProgressInfo);
    if (m_needsAuthorization) {
        m_authHelper->sign(m_job, m_url.pathOrUrl(), m_params);
    }

    connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(recv(KIO::Job*,QByteArray)));
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));

    if (forcedUpdate) {
        connect(m_job, SIGNAL(result(KJob*)), this, SLOT(forceImmediateUpdate()));
    }
    m_job->start();
    return m_job;
}

void TimelineSource::recv(KIO::Job*, const QByteArray& data)
{
    m_xml += data;
}

void TimelineSource::result(KJob *job)
{
    KIO::TransferJob *tj = dynamic_cast<KIO::TransferJob*>(job);
    if (!job || job != m_job) {
        kDebug() << "#fail job is not our job!";
        return;
    }

    //removeAllData();
    //m_imageSource->loadStarted();
    if (job->error()) {
        kDebug() << "job error! : " << job->errorString() << tj->url();// << m_xml;
        // TODO: error handling
    } else {
        //QXmlStreamReader reader(m_xml);
        kDebug() << "Timeline job returned: " << tj->url() << m_xml.count();// << m_xml;
        if (m_requestType == TimelineSource::SearchTimeline) {
            parseJsonSearchResult(m_xml);
        } else {
            //parse(reader);
            parseJson(m_xml);
        }
    }

//      QFile file("/home/sebas/kdesvn/src/declarative-plasmoids/microblog/tmp/output.json");
//      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         QTextStream out(&file);
//         out << m_xml;
//         file.flush();
//         file.close();
//     }

    checkForUpdate();
    m_xml.clear();
    m_job = 0;
}

void TimelineSource::auth(KIO::Job*, const QByteArray& data)
{
    m_oauthTemp += data;
}

void TimelineSource::authFinished(KJob *job)
{
    if (job != m_authJob) {
        return;
    }

    if (job->error()) {
        kDebug() << "Authentication Error " << job->error() << ": " << job->errorText();
    } else {
        QList<QByteArray> pairs = m_oauthTemp.split('&');

        foreach (QByteArray pair, pairs) {
            QList<QByteArray> data = pair.split('=');
            if (data.at(0) == "oauth_token") {
                m_oauthToken = data.at(1);
            } else if (data.at(0) == "oauth_token_secret") {
                m_oauthTokenSecret = data.at(1);
            }
        }
        update(true);
    }
    m_oauthTemp.clear();
    m_authJob = 0;
    //kDebug() << "Authentication succeeded, Token: " << QString(m_oauthToken);
}

void TimelineSource::parseJson(const QByteArray &data)
{
    //kDebug() << "JSON: " << data;
//     kDebug() << "JSON TIMELINE PARSER ONLINE";
    QJson::Parser parser;
    const QVariantList resultsList = parser.parse(data).toList();
    const QVariantMap resultsMap = parser.parse(data).toMap();
    bool hasResult = false;
    int i = 0;
    foreach (const QVariant &v, resultsList) {
        const QVariantMap &tweet = v.toMap();
        i++;
        hasResult = true;
        foreach (const QVariant &k, tweet.keys()) {
            const QString _u = k.toString();
            if (_u != "user") {
                m_tempData[_u] = tweet[_u];
            }
        }

        QString st = tweet["text"].toString();
        if (tweet.keys().contains("entities")) {
            const QVariantMap &entities = tweet["entities"].toMap();
            if (entities.keys().contains("urls")) {
                const QVariantList &urls = entities["urls"].toList();
                foreach (const QVariant &_u, urls) {
                    const QVariantMap u = _u.toMap();
                    const QString &shortUrl = u["url"].toString();
                    const QString &expandedUrl = u["expanded_url"].toString();
                    st.replace(shortUrl, expandedUrl);
                }
            }
        }
        m_tempData["Date"] = tweet["created_at"];
        m_id = tweet["id"].toString();
        m_tempData["Id"] = m_id;
        m_tempData["Status"] = st;
        m_tempData["Source"] = tweet["source"];
        m_tempData["IsFavorite"] = tweet["favorited"];

        parseJsonUser(tweet["user"]);

        //m_tempData["ImageUrl"] = tweet["profile_image_url"];
        if (m_tempData.contains("User")) {
            KUrl url(tweet["profile_image_url"].toString());
            m_imageSource->loadImage(m_tempData["User"].toString(), url);
        }
        emit userFound(tweet["user"], m_serviceBaseUrl.pathOrUrl());

        qulonglong i = m_id.toULongLong();
        //qulonglong o = 0;
        //qulonglong n = 0;
        if (!d->oldestId || d->oldestId > i) {
            d->oldestId = i;
        }
        if (!d->newestId || d->newestId < i) {
            d->newestId = i;
        }
        //kDebug() << "m_id, id, oldest, newest: " << m_id << i << d->oldestId << d->newestId;

        if (!m_id.isEmpty()) {
            QVariant v;
            v.setValue(m_tempData);
//             foreach (const QString &k, m_tempData.keys()) {
//                 kDebug() << "setting data" << m_id << k << m_tempData[k];
//             }
            setData(m_id, v);
            m_id.clear();
        }
    }
    kDebug() << "Found " << i << " Tweets.";
    if (!hasResult) {
        const QVariantList resultsList = parser.parse(data).toList();
        if (!resultsList.count()) {
            const QVariantMap &map = parser.parse(data).toMap();
            const QString &e = map["error"].toString();
            const QString &r = map["request"].toString();
            //kDebug() << "Error? : " << m_xml;
//             kDebug() << "  E: " << e << " " << r;
            m_tempData["Status"] = QVariant(e + " (" + r + ")");
            m_tempData["User"] = QVariant(i18n("Error"));
            QVariant v;
            v.setValue(m_tempData);
            setData("1234", v);

        }
    }
    checkForUpdate();

}

void TimelineSource::parseJsonUser(const QVariant& data)
{
    const QVariantMap &user = data.toMap();
//     foreach (const QVariant &k, user.keys()) {
//         kDebug() << "   user k : " << k << " :: " << user[k.toString()].toString();
//         //m_tempData[k.toString()] = user[k.toString()];
//     }

    // compatibility with old API
    m_tempData["User"] = user["screen_name"];
    m_tempData["ImageUrl"] = user["profile_image_url"].toString();
    //kDebug() << " imageUrl: " << m_tempData["ImageUrl"];
    m_tempData["name"] = user["name"];
    if (m_tempData.contains("ImageUrl")) {
        KUrl url(m_tempData["ImageUrl"].toString());
        m_imageSource->loadImage(m_tempData["User"].toString(), url);
    }
    m_tempData["Url"] = user["url"];

    //kDebug() << "User done";
}


void TimelineSource::parseJsonSearchResult(const QByteArray &data)
{
    QJson::Parser parser;
    const QVariantMap resultsMap = parser.parse(data).toMap();
    bool hasResult = false;
    foreach (QVariant v, resultsMap.keys()) {
        if (v.toString() == "results") {
            foreach (const QVariant &w, resultsMap[v.toString()].toList()) {
                hasResult = true;
                QVariantMap r = w.toMap();

                foreach (const QVariant &k, r.keys()) {
                    const QString _u = k.toString();
                    if (_u != "user") {
                        m_tempData[_u] = r[_u];
                    }
                }
                // QML Model
                //     property string messageId: model["Id"]
                //     property string user: model["User"]
                //     property string source: model["source"]
                //     property string dateTime: model["created_at"]
                //     property bool isFavorite: model["favorited"]
                //     property string message: model["Status"]

                m_tempData["name"] = r["from_user_name"];
                m_tempData["Date"] = r["created_at"];
                m_id = r["id"].toString();
                m_tempData["Id"] = m_id;
                m_tempData["Status"] = r["text"];
                m_tempData["Source"] = r["source"];
                m_tempData["User"] = r["from_user"];
                // not supported in search
                m_tempData["IsFavorite"] = "false";
                m_tempData["retweet_count"] = "0";
                m_tempData["favorited"] = "false";
                m_tempData["ImageUrl"] = r["profile_image_url"];
                if (m_tempData.contains("User")) {
                    KUrl url(r["profile_image_url"].toString());
                    m_imageSource->loadImage(m_tempData["User"].toString(), url);
//                     emit userFound(m_tempData["User"], m_serviceBaseUrl.pathOrUrl());
                }
                if (!m_id.isEmpty()) {
                    QVariant v;
                    v.setValue(m_tempData);
                    setData(m_id, v);
                    m_id.clear();
                }
                qulonglong i = m_id.toULongLong();
                if (!d->oldestId || d->oldestId > i) {
                    d->oldestId = i;
                }
                if (!d->newestId || d->newestId < i) {
                    d->newestId = i;
                }
            }
        }

    }
    if (!hasResult) {
        const QVariantList resultsList = parser.parse(data).toList();
        kDebug() << "Found " << resultsList.count() << " search result tweets";
        if (!resultsList.count()) {
            const QVariantMap &map = parser.parse(data).toMap();
            const QString &e = map["error"].toString();
            const QString &r = map["request"].toString();
            //kDebug() << "Error? : " << m_xml;
            //kDebug() << "  E: " << e << " " << r;
            m_tempData["Status"] = QVariant(e + " (" + r + ")");
            QVariant v;
            v.setValue(m_tempData);
            setData("1234", v);

        }
    }
}

void TimelineSource::setImageSource(ImageSource *source)
{
    m_imageSource = source;
}

ImageSource *TimelineSource::imageSource() const
{
    return m_imageSource;
}

#include <timelinesource.moc>

