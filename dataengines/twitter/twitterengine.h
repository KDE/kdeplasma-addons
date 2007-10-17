/*
 *   Copyright (C) 2007 Trever Fischer <wm161@wm161.net>
 *   Copyright (C) 2007 André Duffeck <duffeck@kde.org>
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
#include <QStringList>

class QDomNodeList;
class QHttp;

class TwitterEngine : public Plasma::DataEngine
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString interval READ interval WRITE setInterval)

    public:
        TwitterEngine(QObject* parent,const QVariantList& args);
        ~TwitterEngine();

        QString password() const;
        void setPassword(const QString &password);

        QString username() const;
        void setUsername(const QString &username);

        QString interval() const;
        void setInterval(const QString& interval);

        //QStringList sources() const;
        enum UpdateType { Timeline=1, Status, UserTimeline, UserTimelineWithFriends, UserImage };

    protected:
        bool sourceRequested(const QString &name);

    protected slots:
        void updateTimeline();
        void updateUser(const QString &who);
        void updateUserWithFriends(const QString &who);
        void getTweet(const int &ID);
        void getUserImage( const QString &who, const KUrl& url );
        void requestFinished(int id, bool error);
        bool updateSource(const QString &source);

private:
        QList<QVariant> parseStatuses(QDomNodeList items);

        QString m_username;
        QString m_password;
        QString m_interval;
        QHttp* m_http;
        QMap<int,UpdateType> m_updates;
        QMap<int,QString> m_timelines;
        QStringList m_activeSources;
        QMap<QString,KUrl> m_userImages;
};

K_EXPORT_PLASMA_DATAENGINE(twitter, TwitterEngine)

#endif
