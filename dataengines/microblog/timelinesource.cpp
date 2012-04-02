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

#include <QXmlStreamReader>
#include <QtCrypto/QtCrypto>

#include <qjson/parser.h>

#include <KDebug>
#include <KIO/Job>

#include "imagesource.h"
#include "koauth.h"

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

//const QString TimelineSource::AccessTokenUrl = "https://api.twitter.com/oauth/access_token";
const QString TimelineSource::AccessTokenUrl = "https://identi.ca/api/oauth/access_token";
//const QString TimelineSource::AppName = "Plasma";

TweetJob::TweetJob(TimelineSource *source, const QString &operation, const QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(source->account(), operation, parameters, parent),
      m_url(source->serviceBaseUrl()),
      m_parameters(parameters),
      m_source(source)
{
    if (operation == "statuses/retweet" ||
        operation == "favorites/create" ||
        operation == "favorites/destroy") {
        m_url.setPath(m_url.path()+QString("%1/%2.xml").arg(operation).arg(parameters.value("id").toString()));
        kDebug() << "Operation" << operation << m_url;
    } else if (operation == "update") {
        m_url.setPath(m_url.path()+QString("statuses/%1.xml").arg(operation));
        kDebug() << "Updating status" << m_url;

    } else {
        m_url.setPath(m_url.path()+operation+".xml");
    }
}

void TweetJob::start()
{
    QByteArray data;

    data = "source=kdemicroblog";
    {
        QMapIterator<QString, QVariant> i(m_parameters);
        while (i.hasNext()) {
            i.next();
            if (!i.value().toString().isEmpty()) {
                if (i.key() == "status") {
                    data = data.append("&status=" + i.value().toString().toUtf8().toPercentEncoding());
                } else {
                    data = data.append(QString("&"+i.key()+"="+i.value().toString()).toLatin1());
                }
            }
        }
    }
    KIO::Job *job = KIO::http_post(m_url, data, KIO::HideProgressInfo);

    KOAuth::ParamMap params;
    params.insert("source", "kdemicroblog");

    {
        QMapIterator<QString, QVariant> i(m_parameters);
        while (i.hasNext()) {
            i.next();
            if (!i.value().toString().isEmpty()) {
                if (i.key() == "status") {
                    params.insert("status", i.value().toString().toUtf8().toPercentEncoding());
                    data = data.append("&status=" + i.value().toString().toUtf8().toPercentEncoding());
                } else {
                    params.insert(i.key().toLatin1(), i.value().toString().toLatin1());
                    data = data.append(QString("&"+i.key()+"="+i.value().toString()).toLatin1());
                }
            }
        }
    }
    m_source->oAuthHelper()->sign(job, m_url.pathOrUrl(), params, KOAuth::POST);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void TweetJob::result(KJob *job)
{
    setError(job->error());
    setErrorText(job->errorText());
    setResult(!job->error());
}

TimelineService::TimelineService(TimelineSource *parent)
    : Plasma::Service(parent),
      m_source(parent)
{
    setName("tweet");
}

Plasma::ServiceJob* TimelineService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    if (operation == "update" || operation == "statuses/retweet" || operation == "favorites/create" || operation == "favorites/destroy") {
        return new TweetJob(m_source, operation, parameters);
    } else if (operation == "refresh") {
        m_source->update(true);
    } else if (operation == "auth") {
        m_source->setPassword(parameters.value("password").toString());
        const QString user = parameters.value("user").toString();
        const QString password = parameters.value("password").toString();
        m_source->startAuthorization(user, password);
    }

    return new Plasma::ServiceJob(m_source->account(), operation, parameters, this);
}

TimelineSource::TimelineSource(const QString &serviceUrl, RequestType requestType, KOAuth::KOAuth *oauthHelper, const QStringList &parameters, QObject* parent)
    : Plasma::DataContainer(parent),
      m_serviceBaseUrl(serviceUrl),
      m_requestType(requestType),
      m_job(0),
      m_authHelper(oauthHelper),
      m_parameters(parameters),
      m_authJob(0),
      m_qcaInitializer(0)
{
//     //who should be something like user@http://twitter.com, if there isn't any @, http://twitter.com will be the default
//     QStringList account = who.split('@');
//     if (account.count() == 2) {
//         m_user = account.at(0);
//         m_serviceBaseUrl = KUrl(account.at(1));
//     } else {
//         kWarning() << "serviceBaseUrl unclear. forcing twitter.com";
//         m_serviceBaseUrl = KUrl("https://twitter.com/");
//     }

    //just create it to correctly initialize QCA and clean up when createSignature() returns
    m_qcaInitializer = new QCA::Initializer(); // FIXME: move into qoautohelper?

//     kDebug() << "authorized." << m_authHelper->serviceBaseUrl()<< m_authHelper->isAuthorized() << m_authHelper->accessToken();
    // set up the url
    QString query;
    switch (m_requestType) {
    case CustomTimeline:
    case SearchTimeline:
        m_params.clear();
        if (m_parameters.count()) {
            m_params.insert("q", m_parameters[0].toLocal8Bit());
        }
        //m_url = KUrl("http://search.twitter.com/search.atom?q=" + parameters.at(0));
        query = QString(QUrl::toPercentEncoding(parameters.at(0).toUtf8()));
        m_url = KUrl("http://search.twitter.com/search.json?rpp=5&include_entities=true&show_user=true&result_type=mixed&q=" + query);
        //kDebug() << "Search or Custom Url: " << m_url << m_serviceBaseUrl;
        break;
   case Profile:
        m_url = KUrl(m_serviceBaseUrl, QString("users/show/%1.xml").arg(parameters.at(0)));
        break;
    case DirectMessages:
        m_url = KUrl(m_serviceBaseUrl, "direct_messages.xml");
        break;
    case Replies:
        m_url = KUrl(m_serviceBaseUrl, "statuses/replies.xml");
        break;
    case TimelineWithFriends:
        m_url = KUrl(m_serviceBaseUrl, "statuses/friends_timeline.xml");
        break;
    case Timeline:
    default:
        m_url = KUrl(m_serviceBaseUrl, "statuses/user_timeline.xml");
        break;
    }

//     kDebug() << "authorized." << m_authHelper->isAuthorized();
    if (m_authHelper->isAuthorized()) {
        update();
    }
    connect(m_authHelper, SIGNAL(authorized()), SLOT(update()));
}

TimelineSource::~TimelineSource()
{
    delete m_qcaInitializer;
}

Plasma::Service* TimelineSource::createService()
{
    return new TimelineService(this);
}

void TimelineSource::startAuthorization(const QString& user, const QString& password)
{
    emit authorize(m_serviceBaseUrl.pathOrUrl(), user, password);
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
    m_authHelper->authorize(m_serviceBaseUrl.pathOrUrl() ,m_user, password);
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

void TimelineSource::update(bool forcedUpdate)
{
    if (!m_authHelper->isAuthorized()) {
        return;
    }
    if (m_job) {
        // We are already performing a fetch, let's not bother starting over
        return;
    }
    //kDebug() << "Creating job..." << m_url;

    // Create a KIO job to get the data from the web service
    m_job = KIO::get(m_url, KIO::Reload, KIO::HideProgressInfo);
    m_authHelper->sign(m_job, m_url.pathOrUrl(), m_params);
//     kDebug() << "signed" << m_url.pathOrUrl();

    connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(recv(KIO::Job*,QByteArray)));
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));

    if (forcedUpdate) {
        connect(m_job, SIGNAL(result(KJob*)), this, SLOT(forceImmediateUpdate()));
    }
    m_job->start();
}

void TimelineSource::recv(KIO::Job*, const QByteArray& data)
{
    //kDebug() << data;
    m_xml += data;
}

void TimelineSource::result(KJob *job)
{
    KIO::TransferJob *tj = dynamic_cast<KIO::TransferJob*>(job);
    if (!job || job != m_job) {
        kDebug() << "#fail job is not our job!";
        return;
    }

    removeAllData();
    //m_imageSource->loadStarted();
    if (job->error()) {
        kDebug() << "job error! : " << job->errorString() << tj->url();
        // TODO: error handling
    } else {
        QXmlStreamReader reader(m_xml);
        if (m_requestType == TimelineSource::SearchTimeline) {
            //kDebug() << "SearchTimeline job returned: " << tj->url() << data().count();// << m_xml;
            if (tj->url().pathOrUrl().contains("atom")) {
                parseSearchResult(reader);
            } else {
                parseJsonSearchResult(m_xml);
            }
        } else {
            parse(reader);
        }
    }

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

void TimelineSource::parse(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            QString tag = xml.name().toString().toLower();

            if (tag == "status") {
                readStatus(xml);
            } else if (tag == "user") {
                readUser(xml);
            } else if (tag == "direct_message") {
                readDirectMessage(xml);
            }
        }
    }

    if (xml.hasError()) {
        kWarning() << "Fatal error on line" << xml.lineNumber()
                   << ", column" << xml.columnNumber() << ":"
                   << xml.errorString();
        m_tempData.clear();
        m_id.clear();
    }
}

void TimelineSource::readStatus(QXmlStreamReader &xml)
{
    m_tempData.clear();

    //kDebug() << "- BEGIN STATUS -" << endl;

    while (!xml.atEnd()) {
        xml.readNext();

        QString tag = xml.name().toString().toLower();

        if (xml.isEndElement() && tag == "status") {
            break;
        }

        if (xml.isStartElement()) {
            QString cdata;

            if (tag == "user") {
                readUser(xml);
            } else {
                cdata = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                //cdata = xml.text();
            }

            if (tag == "created_at") {
                m_tempData["Date"] = cdata;
            } else if (tag == "id") {
                m_tempData["Id"] = cdata;
                m_id = cdata;
            } else if (tag == "text") {
                m_tempData["Status"] = cdata;
            } else if (tag == "source") {
                m_tempData["Source"] = cdata;
            } else if (tag == "favorited") {
                m_tempData["IsFavorite"] = cdata;
            }
        }
    }

    //kDebug() << "- END STATUS -" << endl;

    if (!m_id.isEmpty()) {
        QVariant v;
        v.setValue(m_tempData);
        //kDebug() << "setting data" << m_id << v;
        setData(m_id, v);
        m_id.clear();
    }

    m_tempData.clear();
}

void TimelineSource::parseJsonSearchResult(const QByteArray &data)
{
    //kDebug() << "JSON: " << data;
//     kDebug() << "JSON PARSER ONLINE";
    QJson::Parser parser;
    const QVariantMap resultsMap = parser.parse(data).toMap();
    //QVariantMap res = resultsMap["results"].toMap();
    foreach (QVariant v, resultsMap.keys()) {
        //kDebug() << "QVariantMap" << v.toString() << resultsMap[v.toString()];
        if (v.toString() == "results") {
            //kDebug() << " ################################# " << endl;
                //["results"].toMap()
            foreach (const QVariant &w, resultsMap[v.toString()].toList()) {
                //kDebug() << "---------" << w;
                QVariantMap r = w.toMap();
                m_tempData["Date"] = r["created_at"];
                m_id = r["id"].toString();
                m_tempData["Id"] = m_id;
                m_tempData["Status"] = r["text"];
                m_tempData["Source"] = r["source"];
                //QString u = cdata.split(' ').at(0);
                m_tempData["User"] = r["from_user"];

                // not supported in search
                m_tempData["IsFavorite"] = false;
                m_tempData["ImageUrl"] = r["profile_image_url"];
                if (m_tempData.contains("User")) {
                    KUrl url(r["profile_image_url"].toString());
                    m_imageSource->loadImage(m_tempData["User"].toString(), url);
                }

//                 foreach (const QVariant &x, w.toMap().keys()) {
//                     //kDebug() << "           prop: " << x;
// //                     kDebug() << "  PP " << x.toString() << " : " << w.toMap()[x.toString()].toString();
// 
//                 }

                if (!m_id.isEmpty()) {
                    QVariant v;
                    v.setValue(m_tempData);
                    //foreach (const QString &k, m_tempData.keys()) {
                        //kDebug() << "setting data" << m_id << k << m_tempData[k];
                    //}
                    setData(m_id, v);
                    m_id.clear();
                }
//
            }
        }

    }

//     const QString& match = "duckduckgo";
// 
//     if (match == "define") {
//         //dictionary mode
//         kDebug() << "Heading:" << resultsMap.value("Heading");
//         kDebug() << "AbstractSource:" << resultsMap.value("AbstractSource");
//         kDebug() << "Abstract:" << resultsMap.value("Abstract");
//         kDebug() << "AbstractURL:" << resultsMap.value("AbstractURL");
//     } else if (match == "wolfram") {
//         //wolfram mode (simple redirection, because web search providers are assholes)
//         kDebug() << "Redirect:" << resultsMap.value("Redirect");
//     } else if (match == "duckduckgo") {
//         QList<QVariant> related = resultsMap.value("RelatedTopics").toList();
// 
//         foreach (const QVariant& variant, related) {
//             QVariantMap submap = variant.toMap();
// 
//             kDebug() << "FirstURL:" << submap.value("FirstURL");
//             kDebug() << "Text:" << submap.value("Text");
//             kDebug() << "Icon:" << submap.value("Icon").toMap().value("URL");
//         }
//     }

}



void TimelineSource::parseSearchResult(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            QString tag = xml.name().toString().toLower();
            if (tag == "entry") {
                //kDebug() << " -----------" << tag;
                readSearchStatus(xml);
            }
        }
    }

    if (xml.hasError()) {
        kWarning() << "Fatal error on line" << xml.lineNumber()
                   << ", column" << xml.columnNumber() << ":"
                   << xml.errorString();
        m_tempData.clear();
        m_id.clear();
    }
}

void TimelineSource::readSearchStatus(QXmlStreamReader &xml)
{
    m_tempData.clear();

    //kDebug() << "- BEGIN SEARCH STATUS -" << endl << xml.text();

    while (!xml.atEnd()) {
        xml.readNext();

        QString tag = xml.name().toString().toLower();

        if (xml.isEndElement() && tag == "entry") {
            break;
        }

        if (xml.isStartElement()) {
            QString cdata;

//             if (tag == "user") {
//                 readUser(xml);
//             } else {
//                 cdata = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
//             }
            cdata = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
//             kDebug() << "Tag >.. " << tag << cdata;

            if (tag == "published") {
                m_tempData["Date"] = cdata;
            } else if (tag == "id") {
                m_id = cdata.split(':').at(2);
                m_tempData["Id"] = m_id;
            } else if (tag == "content") {
                m_tempData["Status"] = cdata;
            } else if (tag == "source") {
                m_tempData["Source"] = cdata;
            } else if (tag == "author") {
                QString u = cdata.split(' ').at(0);
                m_tempData["User"] = u;
            }

            // not supported in search
            m_tempData["IsFavorite"] = false;
        }
    }

//     kDebug() << "- END SEARCH STATUS -" << m_id << endl;

    if (!m_id.isEmpty()) {
        QVariant v;
        v.setValue(m_tempData);
//         foreach (const QString &k, m_tempData.keys()) {
            //kDebug() << "setting data" << m_id << k << m_tempData[k];
//         }
        setData(m_id, v);
        m_id.clear();
    }

    m_tempData.clear();
}

void TimelineSource::readUser(QXmlStreamReader &xml, const QString &tagName)
{
    //kDebug() << "- BEGIN USER -" << endl;

    while (!xml.atEnd()) {
        xml.readNext();

        QString tag = xml.name().toString().toLower();

        if (xml.isEndElement() && tag == tagName) {
            break;
        }

        if (xml.isStartElement()) {
            QString cdata;

            if (tag == "status") {
                readStatus(xml);
            } else {
                cdata = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
            }

            if (tag == "screen_name") {
                m_tempData["User"] = cdata;
                if (m_tempData.contains("ImageUrl")) {
                    KUrl url(m_tempData["ImageUrl"].toString());
                    m_imageSource->loadImage(cdata, url);
                }
            } else if (tag == "profile_image_url") {
                m_tempData["ImageUrl"] = cdata;
                if (m_tempData.contains("User")) {
                    KUrl url(cdata);
                    m_imageSource->loadImage(m_tempData["User"].toString(), url);
                }
            } else if (tag ==  "url") {
                m_tempData["Url"] = cdata;
            }
        }
    }

    //kDebug() << "- END USER -" << endl;
}

void TimelineSource::readDirectMessage(QXmlStreamReader &xml)
{
    m_tempData.clear();

    //kDebug() << "- BEGIN DIRECT MESSAGE -" << endl;

    while (!xml.atEnd()) {
        xml.readNext();

        QString tag = xml.name().toString().toLower();

        if (xml.isEndElement() && tag == "direct_message") {
            break;
        }

        if (xml.isStartElement()) {
            QString cdata;

            // Use nested sender user data for avatar and ignore recipient
            if (tag == "sender") {
                readUser(xml, "sender");
            } else if (tag == "recipient") {
                skipTag(xml, "recipient");
            } else {
                cdata = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
            }

            if (tag == "created_at") {
                m_tempData["Date"] = cdata;
            } else if (tag == "id") {
                m_id = cdata;
            } else if (tag == "text") {
                m_tempData["Status"] = cdata;
            }
        }
    }

    //kDebug() << "- END DIRECT MESSAGE -" << endl;

    if (!m_id.isEmpty()) {
        QVariant v;
        v.setValue(m_tempData);
        kDebug() << "setting data" << m_id << v;
        setData(m_id, v);
        m_id.clear();
    }

    m_tempData.clear();
}

void TimelineSource::skipTag(QXmlStreamReader &xml, const QString &tagName)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name() == tagName) {
            return;
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

