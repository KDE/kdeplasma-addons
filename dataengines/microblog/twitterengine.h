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
class ImageSource;

/**
 * Twitter Data Engine
 *
 * This engine provides access to twitter.com timelines.
 * There are two types of timeline you can connect to:
 * Timeline:<user>
 * TimelineWithFriends:<user>
 *
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

    public:
        TwitterEngine(QObject* parent, const QVariantList& args);
        ~TwitterEngine();

        Plasma::Service* serviceForSource(const QString &name);

    protected:
        //from DataEngine
        bool sourceRequestEvent(const QString &name);

    protected slots:
        bool updateSourceEvent(const QString &name);

    private:
	static const QString timelinePrefix;
	static const QString timelineWithFriendsPrefix;
	static const QString profilePrefix;
        ImageSource *m_imageSource;

};

K_EXPORT_PLASMA_DATAENGINE(twitter, TwitterEngine)

#endif
