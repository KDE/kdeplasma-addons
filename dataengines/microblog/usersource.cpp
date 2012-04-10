/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "usersource.h"


#include <KIO/Job>

#include <qjson/parser.h>
#include <QXmlStreamReader>

UserSource::UserSource(const QString &who, const QString &serviceBaseUrl, QObject* parent)
    : Plasma::DataContainer(parent),
      m_user(who),
      m_serviceBaseUrl(serviceBaseUrl)
{
    setObjectName(QLatin1String("User"));
    //emit loadImage(who, KUrl());
}

UserSource::~UserSource()
{
}

void UserSource::loadUserInfo(const QString &who, const QString &serviceBaseUrl)
{
    if (who.isEmpty() || serviceBaseUrl.isEmpty()) {
        return;
    }

    QString _s = serviceBaseUrl;
    if (!_s.endsWith('/')) {
        _s.append('/');
    }
    //const QString u = _s + "users/show/" + who + ".xml";
    const QString u = _s + "users/show/" + who + ".json";
    if (m_currentUrl == u) {
        return;
    }
    m_currentUrl = u;
    kDebug() << "Requesting user info for " << who << " from ... " << u;
    //return;
    //m_runningJobs++;
    kDebug() << "NEW JOB." << who << u;
    KIO::Job *job = KIO::get(u, KIO::NoReload, KIO::HideProgressInfo);
    job->setAutoDelete(true);
    m_jobs[job] = who;
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(recv(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void UserSource::recv(KIO::Job* job, const QByteArray& data)
{
    KIO::TransferJob* kiojob = dynamic_cast<KIO::TransferJob*>(job);
    if (kiojob->url().pathOrUrl() == m_currentUrl) {
        // Only consider the last job started, discard others.
        m_xml += data;
    } else {
        kDebug() << "Discarding data of job" << kiojob->url().pathOrUrl();
    }
}

void UserSource::result(KJob *job)
{
    if (!m_jobs.contains(job)) {
        return;
    }
    KIO::TransferJob* kiojob = dynamic_cast<KIO::TransferJob*>(job);
    //const QString cacheKey = who + "@" + kiojob->url().pathOrUrl();
    if (kiojob->url().pathOrUrl() == m_currentUrl) {
        // Only consider the last job started, discard others.
        if (job->error()) {
            // TODO: error handling
        } else {
            if (kiojob->url().pathOrUrl().contains(".json")) {
                //kDebug() << "parseJson ..." << m_xml;
                parse(m_xml);
            } else {
//                 QXmlStreamReader reader(m_xml);
//                 parse(reader);
            }
            checkForUpdate();
            m_xml.clear();
        }
    } else {
        kDebug() << "Discarding results of job" << kiojob->url().pathOrUrl() << m_currentUrl;
    }
    m_jobs.remove(job);
    m_jobData.remove(job);
    checkForUpdate();
}

void UserSource::parse(QByteArray& jsonData)
{
    QJson::Parser parser;
    //const QVariantList resultsList = parser.parse(data).toList();
    const QVariant &user = parser.parse(jsonData);
    parseJson(user);
}

void UserSource::parseJson(const QVariant &data)
{
    const QVariantMap &user = data.toMap();
    //kDebug() << "JSON: " << data;
//     kDebug() << "JSON USER PARSER ONLINE";
    foreach (const QVariant &k, user.keys()) {
        const QString _k = k.toString();
//         kDebug() << "   user k : " << _k;
        // Exclude last status update for now, would be a QVariantMap though
        if (_k != "status") {
            setData(k.toString(), user[_k]);
        }
    }

    // compatibility with old API
    setData("User", user["screen_name"]);
    if (user["name"].toString().isEmpty()) {
        kDebug() << "empty real name" << user["screen_name"].toString() << user;
        setData("realName", user["screen_name"]);

    } else {
        setData("realName", user["name"]);

    }
    setData("ImageUrl", user["profile_image_url"].toString());
//     setData("tweets", user["screen_name"]);
//     setData("friends", user["screen_name"]);
//     setData("followers", user["screen_name"]);
    if (!user["profile_image_url"].toString().isEmpty()) {
//         kDebug() << "Got image url for: " << user["screen_name"].toString();
    } else {
        kDebug() << " imageUrl: " <<  user["profile_image_url"].toString();
    }
    //if (data().contains("ImageUrl")) {
        //KUrl url(user["profile_image_url"].toString());
        //m_imageSource->loadImage(user["screen_name"].toString(), url);

    //}
    //m_tempData["Url"] = user["url"];

// //     kDebug() << "resultsList.count() :: " << resultsList.count();
//     foreach (const QVariant &v, resultsList) {
//         const QVariantMap &tweet = v.toMap();
// //         kDebug() << " ################################# " << endl;
//         foreach (const QVariant &k, tweet.keys()) {
//             const QString _u = k.toString();
// //             kDebug() << " tweet k : " << _u;
//             if (_u != "user") {
//                 m_tempData[_u] = tweet[_u];
//             }
//         }
// 
//         m_tempData["Date"] = tweet["created_at"];
//         m_id = tweet["id"].toString();
//         m_tempData["Id"] = m_id;
//         m_tempData["Status"] = tweet["text"];
//         m_tempData["Source"] = tweet["source"];
//         m_tempData["IsFavorite"] = tweet["favorited"];
// 
//         parseJsonUser(tweet["user"]);
// 
//         //m_tempData["ImageUrl"] = tweet["profile_image_url"];
//         if (m_tempData.contains("User")) {
//             KUrl url(tweet["profile_image_url"].toString());
//             m_imageSource->loadImage(m_tempData["User"].toString(), url);
//         }
// 
// //                 foreach (const QVariant &x, w.toMap().keys()) {
// //                     //kDebug() << "           prop: " << x;
// // //                     kDebug() << "  PP " << x.toString() << " : " << w.toMap()[x.toString()].toString();
// //
// //                 }
// 
//         if (!m_id.isEmpty()) {
//             QVariant v;
//             v.setValue(m_tempData);
//             //foreach (const QString &k, m_tempData.keys()) {
//                 //kDebug() << "setting data" << m_id << k << m_tempData[k];
//             //}
//             setData(m_id, v);
//             m_id.clear();
//         }
//     }
// 
}

void UserSource::parseJsonStatus(const QVariant& data)
{
    Q_UNUSED(data);
//     const QVariantMap &user = data.toMap();
//     foreach (const QVariant &k, user.keys()) {
//         //kDebug() << "   user k : " << k;
//         //m_tempData[k.toString()] = user[k.toString()];
//     }
// 
//     // compatibility with old API
//     m_tempData["User"] = user["screen_name"];
//     m_tempData["ImageUrl"] = user["profile_image_url"].toString();
//     //kDebug() << " imageUrl: " << m_tempData["ImageUrl"];
//     if (m_tempData.contains("ImageUrl")) {
//         KUrl url(m_tempData["ImageUrl"].toString());
//         m_imageSource->loadImage(m_tempData["User"].toString(), url);
//     }
//     m_tempData["Url"] = user["url"];
// 
    //kDebug() << "User done";
}

// void UserSource::parse(QXmlStreamReader &xml)
// {
//     while (!xml.atEnd()) {
//         xml.readNext();
// 
//         if (xml.isStartElement()) {
//             QString tag = xml.name().toString().toLower();
// 
//             if (tag == "status") {
//                 //readStatus(xml);
//             } else if (tag == "user") {
// //                 kDebug() << "Found user.";
//                 readUser(xml);
//             } else if (tag == "direct_message") {
//                 //readDirectMessage(xml);
//             }
//         }
//     }
// 
//     if (xml.hasError()) {
//         kWarning() << "Fatal error on line" << xml.lineNumber()
//                    << ", column" << xml.columnNumber() << ":"
//                    << xml.errorString();
//     }
// }
// 
// void UserSource::readUser(QXmlStreamReader &xml)
// {
//     QHash<QString, QString> tagKeys;
//     tagKeys.insert("id", "userid");
//     tagKeys.insert("screen_name", "username");
//     tagKeys.insert("name", "realname");
//     tagKeys.insert("location", "location");
//     tagKeys.insert("description", "description");
//     tagKeys.insert("protected", "protected");
//     tagKeys.insert("followers_count", "followers");
//     tagKeys.insert("friends_count", "friends");
//     tagKeys.insert("statuses_count", "tweets");
//     tagKeys.insert("time_zone", "timezone");
//     tagKeys.insert("utc_offset", "utcoffset");
//     tagKeys.insert("profile_image_url", "profileimageurl");
//     tagKeys.insert("statusnet:profile_url", "profileurl");
//     tagKeys.insert("url", "url");
//     tagKeys.insert("following", "following");
//     tagKeys.insert("notifications", "notifications");
//     tagKeys.insert("statusnet:blocking", "blocking");
//     tagKeys.insert("created_at", "created");
// 
//     //kDebug() << "- BEGIN USER -" << endl;
//     const QString tagName("user");
// 
//     while (!xml.atEnd()) {
//         xml.readNext();
//         //kDebug() << "next el";
//         QString tag = xml.name().toString().toLower();
// 
//         if (xml.isEndElement() && tag == tagName) {
//             break;
//         }
// 
//         if (xml.isStartElement()) {
//             QString cdata;
// 
//             if (tag == "status") {
//                 //readStatus(xml);
//             } else {
//                 cdata = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
//                 if (tagKeys.keys().contains(tag)) {
//                     setData(tagKeys[tag], cdata);
//                 }
//             }
//         }
//     }
// 
//     // Make sure our avatar is loaded
// //     kDebug() << "requesting profile pic" << data()["username"] << data()["profileimageurl"];
//     //const QString who = 
//     emit loadImage(data()["username"].toString(), data()["profileimageurl"].toUrl());
// //     kDebug() << " read user: " << data()["username"].toString();
//     //kDebug() << "- END USER -" << endl;
// }

#include <usersource.moc>

