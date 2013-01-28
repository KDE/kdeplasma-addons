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


TweetJob::TweetJob(TimelineSource *source, const QString &operation, const QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(source->account(), operation, parameters, parent),
      m_url(source->serviceBaseUrl()),
      m_parameters(parameters),
      m_source(source),
      m_operation(operation)
{

    if (operation == "statuses/retweet" ||
        operation == "favorites/create" ||
        operation == "favorites/destroy") {
        m_url.setPath(m_url.path()+QString("%1/%2.xml").arg(operation).arg(parameters.value("id").toString()));
        kDebug() << "Operation" << operation << m_url;
    
    } else if (operation == "friendships/create" || operation == "friendships/destroy") {
        m_url.setPath(m_url.path()+operation+".json");
        kDebug() << "follow/unfollow: " << m_url;
    } else if (operation == "update") {
        m_url.setPath(m_url.path()+QString("statuses/%1.xml").arg(operation));
        kDebug() << "Updating status" << m_url;
    } else {
        m_url.setPath(m_url.path()+operation+".xml");
    }
}

void TweetJob::start()
{
    kDebug() << "starting job" << m_url;
    QByteArray data;
    QOAuth::ParamMap params;

    data = "source=kdemicroblog";
    params.insert("source", "kdemicroblog");
    {
        QMapIterator<QString, QVariant> i(m_parameters);
        while (i.hasNext()) {
            i.next();
            if (!i.value().toString().isEmpty()) {
                if (i.key() == "status") {
                    const QByteArray status = i.value().toString().toUtf8().toPercentEncoding();
                    params.insert("status", status);
                    data = data.append("&status=" + status);
                } else {
                    const QByteArray key = i.key().toLatin1();
                    const QByteArray value = i.value().toString().toLatin1();
                    params.insert(key, value);
                    data = data.append("&"+key+"="+value);
                }
            }
        }
    }
    KIO::Job *job = KIO::http_post(m_url, data, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");

    m_source->oAuthHelper()->sign(job, m_url.pathOrUrl(), params, KOAuth::POST);
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(recv(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void TweetJob::recv(KIO::Job* , const QByteArray& data)
{
    kDebug() << "data " << m_url;
    m_data.append(data);
}

void TweetJob::result(KJob *job)
{
    kDebug() << "job returned " << m_url;
    kDebug() << "Job returned... e:" << job->errorText();
    //kDebug() << "Job returned data:" << m_data;
    if (m_operation.startsWith("friendships")) {
        kDebug() << "emitting userdata";
        emit userData(m_data);
    }
    setError(job->error());
    setErrorText(job->errorText());
    setResult(!job->error());
    m_data.clear();
}


#include <tweetjob.moc>

