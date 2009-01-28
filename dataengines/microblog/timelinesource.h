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

#ifndef TIMELINESOURCE_H
#define TIMELINESOURCE_H

#include <QList>
#include <QPair>
#include <QXmlDefaultHandler>

#include <KUrl>

#include <Plasma/DataContainer>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

// forward declarations
class KJob;

namespace KIO
{
    class Job;
} // namespace KIO

class TimelineSource;
class ImageSource;

class TweetJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    TweetJob(TimelineSource *source, const QMap<QString, QVariant> &parameters, QObject *parent = 0);
    void start();

private slots:
    void result(KJob *job);

private:
    KUrl m_url;
};

class TimelineService : public Plasma::Service
{
    Q_OBJECT

public:
    TimelineService(TimelineSource *parent);

protected:
    Plasma::ServiceJob* createJob(const QString &operation, QMap<QString, QVariant> &parameters);

private:
    TimelineSource *m_source;
};

class TimelineSource : public Plasma::DataContainer, QXmlDefaultHandler
{
    Q_OBJECT

public:
    enum RequestType {
        Timeline = 0,
        TimelineWithFriends,
        Profile
    };

    TimelineSource(const QString &who, RequestType requestType, QObject* parent);
    ~TimelineSource();

    void update();
    void setPassword(const QString &password);
    QString account() const;
    QString password() const;
    KUrl serviceBaseUrl() const;

    Plasma::Service *createService();

    bool startElement(const QString &namespaceURI, const QString & localName,
                      const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool characters(const QString &ch);
    bool fatalError(const QXmlParseException & exception);
    ImageSource *imageSource() const;
    void setImageSource(ImageSource *);

private slots:
    void recv(KIO::Job*, const QByteArray& data);
    void result(KJob*);

private:
    KUrl m_url;
    KUrl m_serviceBaseUrl;
    ImageSource *m_imageSource;
    QByteArray m_xml;
    QString m_cdata;
    Plasma::DataEngine::Data m_tempData;
    KIO::Job * m_job;
    QString m_id;
};

#endif

