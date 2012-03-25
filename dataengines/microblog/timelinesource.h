/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
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


#include <KUrl>

#include <Plasma/DataContainer>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

// forward declarations
class QXmlStreamReader;
class QOAuthHelper;

class KJob;

namespace KIO
{
    class Job;
} // namespace KIO

namespace QCA
{
    class Initializer;
}

class TimelineSource;
class ImageSource;

class TweetJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    TweetJob(TimelineSource *source, const QString &operation, const QMap<QString, QVariant> &parameters, QObject *parent = 0);
    void start();

private slots:
    void result(KJob *job);

private:
    KUrl m_url;
    QMap<QString, QVariant> m_parameters;
    TimelineSource *m_source;
};

class TimelineService : public Plasma::Service
{
    Q_OBJECT

public:
    TimelineService(TimelineSource *parent);

//Q_SIGNALS:
    //authorize(const QString &password);

protected:
    Plasma::ServiceJob* createJob(const QString &operation, QMap<QString, QVariant> &parameters);

private:
    TimelineSource *m_source;
};

class TimelineSource : public Plasma::DataContainer
{
    Q_OBJECT

public:
    enum RequestType {
        Timeline = 0,
        TimelineWithFriends,
        Replies,
        DirectMessages,
        Profile,
        User
    };

    TimelineSource(const QString &who, RequestType requestType, QOAuthHelper *oauthHelper, QObject* parent);
    ~TimelineSource();

    void setPassword(const QString &password);
    QString account() const;
    QString password() const;
    KUrl serviceBaseUrl() const;

    bool useOAuth() const;
    QByteArray oauthToken() const;
    QByteArray oauthTokenSecret() const;
    void setOAuthHelper(QOAuthHelper *authHelper);
    QOAuthHelper* oAuthHelper();

    Plasma::Service* createService();
    void startAuthorization(const QString &password);

    ImageSource* imageSource() const;
    void setImageSource(ImageSource *);

public Q_SLOTS:
    void update(bool forcedUpdate = false);

Q_SIGNALS:
    void authorize(const QString &serviceBaseUrl, const QString &user, const QString &password);

private slots:
    void recv(KIO::Job*, const QByteArray& data);
    void result(KJob*);

    void auth(KIO::Job*, const QByteArray& data);
    void authFinished(KJob*);

private:
    void parse(QXmlStreamReader &xml);
    void readStatus(QXmlStreamReader &xml);
    void readUser(QXmlStreamReader &xml, const QString &tagName = "user");
    void readDirectMessage(QXmlStreamReader &xml);
    void skipTag(QXmlStreamReader &xml, const QString &tagName);

    // OAuth constants
    static const QString AccessTokenUrl;

    KUrl m_url;
    KUrl m_serviceBaseUrl;
    ImageSource *m_imageSource;
    QByteArray m_xml;
    Plasma::DataEngine::Data m_tempData;
    KIO::Job * m_job;
    QString m_id;

    bool m_useOAuth;
    QOAuthHelper *m_authHelper;
    QString m_user;
    QByteArray m_oauthTemp;
    KIO::Job *m_authJob;
    QByteArray m_oauthToken;
    QByteArray m_oauthTokenSecret;
    QCA::Initializer *m_qcaInitializer;
};

#endif
