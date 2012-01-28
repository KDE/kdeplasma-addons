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

#include <QXmlStreamReader>

UserSource::UserSource(const QString &who, const QString &serviceBaseUrl, QObject* parent)
    : Plasma::DataContainer(parent),
      m_user(who),
      m_serviceBaseUrl(serviceBaseUrl)
{
    setObjectName(QLatin1String("User"));
    loadUserInfo(who, m_serviceBaseUrl);
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
    const QString u = _s + "users/show/" + who + ".xml";
    if (m_currentUrl == u) {
        return;
    }
    m_currentUrl = u;
//     kDebug() << "Requesting user info for " << who << " from ... " << u;
    //return;
    //m_runningJobs++;
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
            QXmlStreamReader reader(m_xml);
            parse(reader);
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


void UserSource::parse(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            QString tag = xml.name().toString().toLower();

            if (tag == "status") {
                //readStatus(xml);
            } else if (tag == "user") {
//                 kDebug() << "Found user.";
                readUser(xml);
            } else if (tag == "direct_message") {
                //readDirectMessage(xml);
            }
        }
    }

    if (xml.hasError()) {
        kWarning() << "Fatal error on line" << xml.lineNumber()
                   << ", column" << xml.columnNumber() << ":"
                   << xml.errorString();
    }
}

void UserSource::readUser(QXmlStreamReader &xml)
{
    QHash<QString, QString> tagKeys;
    tagKeys.insert("id", "userid");
    tagKeys.insert("screen_name", "username");
    tagKeys.insert("name", "realname");
    tagKeys.insert("location", "location");
    tagKeys.insert("description", "description");
    tagKeys.insert("protected", "protected");
    tagKeys.insert("followers_count", "followers");
    tagKeys.insert("friends_count", "friends");
    tagKeys.insert("statuses_count", "tweets");
    tagKeys.insert("time_zone", "timezone");
    tagKeys.insert("utc_offset", "utcoffset");
    tagKeys.insert("profile_image_url", "profileimageurl");
    tagKeys.insert("statusnet:profile_url", "profileurl");
    tagKeys.insert("url", "url");
    tagKeys.insert("following", "following");
    tagKeys.insert("notifications", "notifications");
    tagKeys.insert("statusnet:blocking", "blocking");
    tagKeys.insert("created_at", "created");

    //kDebug() << "- BEGIN USER -" << endl;
    const QString tagName("user");

    while (!xml.atEnd()) {
        xml.readNext();
        //kDebug() << "next el";
        QString tag = xml.name().toString().toLower();

        if (xml.isEndElement() && tag == tagName) {
            break;
        }

        if (xml.isStartElement()) {
            QString cdata;

            if (tag == "status") {
                //readStatus(xml);
            } else {
                cdata = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
                if (tagKeys.keys().contains(tag)) {
                    setData(tagKeys[tag], cdata);
                }
            }
        }
    }

    // Make sure our avatar is loaded
//     kDebug() << "requesting profile pic" << data()["username"] << data()["profileimageurl"];
    //const QString who = 
    emit loadImage(data()["username"].toString(), data()["profileimageurl"].toUrl());
//     kDebug() << " read user: " << data()["username"].toString();
    //kDebug() << "- END USER -" << endl;
}

#include <usersource.moc>

