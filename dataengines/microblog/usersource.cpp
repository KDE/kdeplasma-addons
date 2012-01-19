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
      //m_runningJobs(0),
      m_user(who),
      m_serviceBaseUrl(serviceBaseUrl)
{
    setObjectName(QLatin1String("User"));

    const QString u = QString("%1/users/show/%2.xml").arg(serviceBaseUrl, who);
    kDebug() << "Yo Yo Yo ... " << u << who << serviceBaseUrl;
    loadUserInfo(who, u);
}

UserSource::~UserSource()
{
}

void UserSource::loadStarted()
{
    m_cachedData = data();
    removeAllData();
}

void UserSource::loadFinished()
{
    m_cachedData.clear();
}

void UserSource::loadUserInfo(const QString &who, const KUrl &url)
{
    if (m_cachedData.contains(who)) {
        kDebug() << "UserInfo:" << who;
        setData(who, m_cachedData.value(who));
    }
    //return;
    //m_runningJobs++;
    KIO::Job *job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    job->setAutoDelete(true);
    m_jobs[job] = who;
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(recv(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void UserSource::recv(KIO::Job* job, const QByteArray& data)
{
    //m_jobData[job] += data;
    kDebug() << "   XML: " << data;
    m_xml += data;

    //kDebug() << m_data;
}

void UserSource::result(KJob *job)
{
    kDebug() << "JOB RETURNING.";
    if (!m_jobs.contains(job)) {
        return;
    }

    if (job->error()) {
        // TODO: error handling
    } else {
//         //kDebug() << "done!" << m_jobData;
//         QImage img;
//         img.loadFromData(m_jobData.value(job));
//         kDebug() << "User:" << m_jobs.value(job);
//         setData(m_jobs.value(job), img);
//         emit dataChanged();
        kDebug() << "parsing through .." << m_xml;
        QXmlStreamReader reader(m_xml);
        parse(reader);
        checkForUpdate();
        m_xml.clear();
        kDebug() << "cleared";
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
                kDebug() << "Found user.";
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
        m_cachedData.clear();
        //m_id.clear();
    }
}

void UserSource::readUser(QXmlStreamReader &xml)
{
    QHash<QString, QString> tagKeys;
    tagKeys.insert("screen_name", "User");
    tagKeys.insert("name", "Name");
    tagKeys.insert("location", "Location");
    tagKeys.insert("description", "Description");
    tagKeys.insert("protected", "Description");
    tagKeys.insert("followers_count", "Followers");
    tagKeys.insert("friends_count", "Friends");
    tagKeys.insert("statuses_count", "Tweets");
    tagKeys.insert("time_zone", "Timezone");
    tagKeys.insert("following", "Following");
    tagKeys.insert("notifications", "Notifications");
    tagKeys.insert("statusnet:blocking", "Blocking");
    tagKeys.insert("created_at", "Created");
    
    kDebug() << "- BEGIN USER -" << endl;
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

    kDebug() << "- END USER -" << endl;
}

Plasma::DataEngine::Data UserSource::data()
{
    return m_cachedData;
}


#include <usersource.moc>

