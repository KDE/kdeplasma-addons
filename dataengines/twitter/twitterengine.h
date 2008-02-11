/*
 *   Copyright (C) 2007 Trever Fischer <wm161@wm161.net>
 *   Copyright (C) 2007 André Duffeck <duffeck@kde.org>
 *   Copyright (C) 2007 Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef TWITTERENGINE_H
#define TWITTERENGINE_H

#include <plasma/dataengine.h>

#include <QMap>

class QDomDocument;
class QDomNodeList;
class QHttp;

/**
 * Twitter Data Engine
 *
 * This engine provides access to twitter.com timelines.
 * There are three types of timeline you can connect to:
 * Timeline
 * Timeline:<user>
 * TimelineWithFriends:<user>
 *
 * Timeline is the public timeline that shows tweets from all twitter users.
 * Timeline:<user> shows only the tweets of that user. it currently requires a password.
 * TimelineWithFriends:<user> shows the normal timeline of that user and their
 * friends. it requires a password to be configured for the user.
 *
 * To configure a password, use setConfig(user, password).
 *
 * For each source you connect to, you should also connect to Error:<source> to see http errors.
 * You should also connect to the Error source for general errors.
 *
 * If you need to display user images, the UserImages source provides a list of
 * all images, and LatestImage provides just the most recently downloaded one.
 **/
class TwitterEngine : public Plasma::DataEngine
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus)
    Q_PROPERTY(Plasma::DataEngine::Data config READ config WRITE setConfig)

    public:
        TwitterEngine(QObject* parent, const QVariantList& args);
        ~TwitterEngine();

        QString status() const;
        void setStatus(const QString& refresh);
        Plasma::DataEngine::Data config() const;
        void setConfig(const Plasma::DataEngine::Data& config);

        enum UpdateType { Timeline=1, UserTimeline, UserTimelineWithFriends, UserImage, Post, UserInfo };

    protected:
        //from DataEngine
        bool sourceRequested(const QString &name);

    protected slots:
        void requestFinished(int id, bool error);
        void anonRequestFinished(int id, bool error);
        bool updateSource(const QString &source);

    private:
        void updateTimeline();
        void updateUser(const QString &who);
        void updateUserWithFriends(const QString &who);
        void getUserImage( const QString &who, const KUrl& url );
        void getUserInfo( const QString &who );
        void parseStatuses(QDomNodeList items, const QString& source);
        void parseUserInfo(const QDomDocument &info);

        QString m_status;
        QHttp* m_http;
        QHttp* m_anonHttp;
        QMap<int,UpdateType> m_pendingRequests;
        QMap<int,UpdateType> m_pendingAnonRequests;
        QMap<int,QString> m_pendingNames;
        QMap<QString,KUrl> m_userImages;
        Plasma::DataEngine::Data m_config;
};

K_EXPORT_PLASMA_DATAENGINE(twitter, TwitterEngine)

#endif
