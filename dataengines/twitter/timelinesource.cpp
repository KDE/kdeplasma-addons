/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include <QBuffer>

#include <KIO/Job>

#include "imagesource.h"

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

TweetJob::TweetJob(TimelineSource *source, const QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(source->account(), "update", parameters, parent),
      m_url("http://twitter.com/statuses/update.xml")
{
    m_url.addQueryItem("status", parameters.value("status").toString());
    m_url.addQueryItem("source", "kdetwitter");
    m_url.setUser(source->account());
    m_url.setPass(source->password());
}

void TweetJob::start()
{
    KIO::Job *job = KIO::get(m_url, KIO::Reload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void TweetJob::result(KJob *job)
{
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
    }

    if (operation == "auth") {
        m_source->setPassword(parameters.value("password").toString());
    }

    // fail!
    return new Plasma::ServiceJob(m_who, operation, parameters, this);
}

TimelineSource::TimelineSource(const QString &who, bool includeFriends, QObject* parent)
    : Plasma::DataContainer(parent)
{
    // set up the url
    m_url.setUser(who);
    if (includeFriends) {
        m_url = KUrl("http://twitter.com/statuses/friends_timeline.xml");
    } else {
        m_url = KUrl("http://twitter.com/statuses/user_timeline.xml");
    }

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
    m_url.setPass(password);
}

QString TimelineSource::password() const
{
    return m_url.pass();
}

QString TimelineSource::account() const
{
    return m_url.user();
}

void TimelineSource::update()
{
    if (m_job) {
        // We are already performing a fetch, let's not bother starting over
        return;
    }

    // Create a KIO job to get the data from the web service
    m_job = KIO::get(m_url, KIO::Reload, KIO::HideProgressInfo);
    connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(recv(KIO::Job*, const QByteArray&)));
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void TimelineSource::recv(KIO::Job*, const QByteArray& data)
{
    m_xml += data;
    //kDebug() << m_data;
}

void TimelineSource::result(KJob *job)
{
    if (job != m_job) {
        return;
    }

    removeAllData();
    if (job->error()) {
        // TODO: error handling
    } else {
        //kDebug() << "done!" << m_data;
        QXmlSimpleReader reader;
        reader.setContentHandler(this);
        reader.setErrorHandler(this);
        QBuffer input(&m_xml);
        QXmlInputSource source(&input);
        reader.parse(&source, false);
        m_xml.clear();
    }

    checkForUpdate();
    m_job = 0;
}

bool TimelineSource::startElement(const QString &namespaceURI, const QString &localName,
                                  const QString &qName, const QXmlAttributes &atts)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(qName)
    Q_UNUSED(atts)

    QString tag = localName.toLower();
    if (tag == "status") {
        m_tempData.clear();
        m_id.clear();
    }

    return true;
}

bool TimelineSource::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(qName)

    QString tag = localName.toLower();

    if (tag == "status") {
        if (!m_id.isEmpty()) {
            QVariant v;
            v.setValue(m_tempData);
            setData(m_id, v);
            m_id.clear();
        }

        m_tempData.clear();
    } else if (tag == "id") {
        m_id = m_cdata;
    } else if (tag == "text") {
        m_tempData["Status"] = m_cdata;
    } else if (tag == "profile_image_url") {
        m_tempData["ImageUrl"] = m_cdata;
        if (m_tempData.contains("User")) {
            KUrl url(m_cdata);
            ImageSource::self()->loadImage(m_tempData["User"].toString(), url);
        }
    } else if (tag == "screen_name") {
        m_tempData["User"] = m_cdata;
        if (m_tempData.contains("ImageUrl")) {
            KUrl url(m_tempData["ImageUrl"].toString());
            ImageSource::self()->loadImage(m_cdata, url);
        }
    } else if (tag == "source") {
        m_tempData["Source"] = m_cdata;
    } else if (tag == "url") {
        m_tempData["Url"] = m_cdata;
    }

    m_cdata.clear();
    return true;
}

bool TimelineSource::characters(const QString &ch)
{
    m_cdata.append(ch);
    return true;
}

bool TimelineSource::fatalError(const QXmlParseException &exception)
{
    kWarning() << "Fatal error on line" << exception.lineNumber()
               << ", column" << exception.columnNumber() << ":"
               << exception.message();
    m_tempData.clear();
    m_id.clear();
    return false;
}

#include <timelinesource.moc>

