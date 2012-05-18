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

class UserSourcePrivate {

public:
    UserSourcePrivate()
    {
    }

    QString user;
    QString serviceBaseUrl;
};

UserSource::UserSource(const QString &who, const QString &serviceBaseUrl, QObject* parent)
    : Plasma::DataContainer(parent),
      m_user(who),
      m_serviceBaseUrl(serviceBaseUrl)
{
    d = new UserSourcePrivate();
    d->user = who;
    d->serviceBaseUrl = serviceBaseUrl;
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
//     kDebug() << "Requesting user info for " << who << " from ... " << u;
    //return;
    //m_runningJobs++;
//     kDebug() << "NEW JOB." << who << u;
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
//     kDebug() << "job returned" << m_currentUrl;
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

void UserSource::parse(const QByteArray& jsonData)
{
//     kDebug() << "User parse()" << jsonData;
    QJson::Parser parser;
    //const QVariantList resultsList = parser.parse(data).toList();
    const QVariant &user = parser.parse(jsonData);
    parseJson(user);
}

void UserSource::parseJson(const QVariant &data)
{
    const QVariantMap &user = data.toMap();
    foreach (const QVariant &k, user.keys()) {
        const QString _k = k.toString();
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
//         kDebug() << " user: " << user["name"].toString();

    }
    const QString imgUrl = user["profile_image_url"].toString();
    const QString userName = user["screen_name"].toString();
    setData("ImageUrl", imgUrl);
    if (!imgUrl.isEmpty()) {
//         kDebug() << "Got image url for: " << userName << imgUrl;
        emit loadImage(userName, KUrl(imgUrl));
    }
    checkForUpdate();
}

void UserSource::parseJsonStatus(const QVariant& data)
{
    Q_UNUSED(data);
}


#include <usersource.moc>

