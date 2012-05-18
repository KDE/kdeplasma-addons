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

#include "tweetjob.h"
#include "timelineservice.h"
#include "timelinesource.h"

#include <KDebug>
#include <KIO/Job>

#include "koauth.h"

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)


TimelineService::TimelineService(TimelineSource *parent)
    : Plasma::Service(parent),
      m_source(parent)
{
    setName("tweet");
    kDebug();
}

Plasma::ServiceJob* TimelineService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    kDebug() << "Create Job: " << operation;
    if (operation == "update" || operation == "statuses/retweet" ||
            operation == "favorites/create" || operation == "favorites/destroy" ||
            operation == "friendships/create" || operation == "friendships/destroy") {
        TweetJob *tj = new TweetJob(m_source, operation, parameters);
//         TimelineSource *src = qobject_cast<TimelineSource*>(parent());
//         if (src) {
            kDebug() << "Source found" << m_source;
            connect(tj, SIGNAL(userData(const QByteArray&)), m_source, SIGNAL(userData(const QByteArray&)));
//         }
        return tj;
    } else if (operation == "refresh") {
        Plasma::ServiceJob *sjob = new Plasma::ServiceJob(m_source->account(), operation, parameters, this);
        KIO::Job *getJob = m_source->update(true);
        if (getJob) {
            connect(getJob, SIGNAL(result(KJob*)), sjob, SIGNAL(finished(KJob*)));
        }
        return sjob;
    } else if (operation == "auth") {
        //m_source->setPassword(parameters.value("password").toString());
        const QString user = parameters.value("user").toString();
        const QString password = parameters.value("password").toString();
        kDebug() << "Start authorization for " << user << password;
        m_source->startAuthorization(user, password);
    } else if (operation == "forget") {
        const QString user = parameters.value("user").toString();
        const QString serviceUrl = parameters.value("serviceUrl").toString();
        kDebug() << "Forgetting acount " << user << serviceUrl;
        m_source->forgetAccount(user, serviceUrl);
    } else if (operation == "loadMore") {
        Plasma::ServiceJob *sjob = new Plasma::ServiceJob(m_source->account(), operation, parameters, this);
        kDebug() << "load more ";
        KIO::Job *getJob = m_source->loadMore();
        if (getJob) {
            connect(getJob, SIGNAL(result(KJob*)), sjob, SIGNAL(finished(KJob*)));
        }
        return sjob;
    }


    return new Plasma::ServiceJob(m_source->account(), operation, parameters, this);
}


#include <timelineservice.moc>

