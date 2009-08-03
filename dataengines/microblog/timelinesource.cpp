/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
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

#include <KIO/Job>

#include "imagesource.h"

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

TweetJob::TweetJob(TimelineSource *source, const QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(source->account(), "update", parameters, parent),
      m_url(source->serviceBaseUrl(), "statuses/update.xml")
{
    m_url.addQueryItem("status", parameters.value("status").toString());
    m_url.addQueryItem("source", "kdemicroblog");
    m_url.setUser(source->account());
    m_url.setPass(source->password());
}

void TweetJob::start()
{
    KIO::Job *job = KIO::http_post(m_url, 0, KIO::HideProgressInfo);
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
    if (operation == "update") {
        return new TweetJob(m_source, parameters);
    } else if (operation == "refresh") {
        m_source->update(true);
    } else if (operation == "auth") {
        m_source->setPassword(parameters.value("password").toString());
    }

    return new Plasma::ServiceJob(m_source->account(), operation, parameters, this);
}

TimelineSource::TimelineSource(const QString &who, RequestType requestType, QObject* parent)
    : Plasma::DataContainer(parent),
      m_job(0)
{
    //who should be something like user@http://twitter.com, if there isn't any @, http://twitter.com will be the default
    QStringList account = who.split('@');
    if (account.count() == 2) {
        m_serviceBaseUrl = KUrl(account.at(1));
    } else {
        m_serviceBaseUrl = KUrl("https://twitter.com/");
    }

    // set up the url
    switch (requestType) {
    case Profile:
        m_url = KUrl(m_serviceBaseUrl, QString("users/show/%1.xml").arg(account.at(0)));
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

    m_url.setUser(account.at(0));
    // .. and now actually get the data
    update();
}

TimelineSource::~TimelineSource()
{
}

Plasma::Service* TimelineSource::createService()
{
    return new TimelineService(this);
}

void TimelineSource::setPassword(const QString &password)
{
    bool force = !m_url.password().isEmpty();
    m_url.setPass(password);
    update(force);
}

QString TimelineSource::password() const
{
    return m_url.pass();
}

QString TimelineSource::account() const
{
    return m_url.user();
}

KUrl TimelineSource::serviceBaseUrl() const
{
    return m_serviceBaseUrl;
}

void TimelineSource::update(bool forcedUpdate)
{
    if (m_job || (!account().isEmpty() && password().isEmpty())) {
        // We are already performing a fetch, let's not bother starting over
        //kDebug() << "already updating....." << m_job << account().isEmpty() << password().isEmpty();
        return;
    }

    //kDebug() << "starting an update";
    // Create a KIO job to get the data from the web service
    m_job = KIO::get(m_url, KIO::Reload, KIO::HideProgressInfo);
    connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(recv(KIO::Job*, const QByteArray&)));
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));

    if (forcedUpdate) {
        //kDebug() << "forcing update";
        connect(m_job, SIGNAL(result(KJob*)), this, SLOT(forceImmediateUpdate()));
    }
}

void TimelineSource::recv(KIO::Job*, const QByteArray& data)
{
    m_xml += data;
    //kDebug() << m_data;
}

void TimelineSource::result(KJob *job)
{
    if (job != m_job) {
        //kDebug() << "fail! job is not our job!";
        return;
    }

    removeAllData();
    if (job->error()) {
        //kDebug() << "job error!";
        // TODO: error handling
    } else {
        //kDebug() << "done!" << data().count() << m_xml;
        QXmlStreamReader reader(m_xml);
        parse(reader);
        //kDebug() << "parsing through .." << data().count();
    }

    checkForUpdate();
    m_xml.clear();
    m_job = 0;
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
                cdata = xml.readElementText().trimmed();
            }

            if (tag == "created_at") {
                m_tempData["Date"] = cdata;
            } else if (tag == "id") {
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
                cdata = xml.readElementText().trimmed();
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
                cdata = xml.readElementText().trimmed();
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
        //kDebug() << "setting data" << m_id << v;
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

