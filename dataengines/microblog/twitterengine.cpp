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

#include "twitterengine.h"


#include <KDebug>
#include <KIcon>
#include <ksocketfactory.h>

#include "timelinesource.h"
#include "imagesource.h"
#include "usersource.h"

const QString TwitterEngine::timelinePrefix("Timeline:");
const QString TwitterEngine::timelineWithFriendsPrefix("TimelineWithFriends:");
const QString TwitterEngine::profilePrefix("Profile:");
const QString TwitterEngine::repliesPrefix("Replies:");
const QString TwitterEngine::messagesPrefix("Messages:");
const QString TwitterEngine::userPrefix("User:");

TwitterEngine::TwitterEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    //setMinimumPollingInterval(2 * 60 * 1000); // 2 minutes minimum
    setData("Defaults", "UserImage", KIcon("camera-photo").pixmap(256, 256).toImage());
}

TwitterEngine::~TwitterEngine()
{
}

bool TwitterEngine::sourceRequestEvent(const QString &name)
{
    if (name.startsWith("UserImages:")) {
        // these are updated by the engine itself, not consumers
        kDebug() << " user image req'ed, doing nothing: " << name;
        return true;
    }

    if (!name.startsWith(timelinePrefix) && !name.startsWith(timelineWithFriendsPrefix)
        && !name.startsWith(profilePrefix) && !name.startsWith(repliesPrefix)
        && !name.startsWith(messagesPrefix) && !name.startsWith(userPrefix)) {
        return false;
    }

    kDebug() << "loading: " << name;
    kDebug() << sources();
    //KIcon("meeting-chair").pixmap(256, 256).toImage().save("/tmp/userimage.png");
    scheduleSourcesUpdated();
    //kDebug() << "image added" << sources();

    updateSourceEvent(name); //start a download
    return true;
}

Plasma::Service* TwitterEngine::serviceForSource(const QString &name)
{
    TimelineSource *source = dynamic_cast<TimelineSource*>(containerForSource(name));
    kDebug() << "Service name: " << name;
    if (!source) {
        kDebug() << "source not there.";
        return Plasma::DataEngine::serviceForSource(name);
    }


    Plasma::Service *service = source->createService();
    service->setParent(this);
    return service;
}

//called when it's time to update a source
//also called by twitter from sourceRequested
//and when it thinks an update would be useful
//always returns false because everything is async
bool TwitterEngine::updateSourceEvent(const QString &name)
{
    //kDebug() << name;
    //right now it only makes sense to do an update on timelines
    if (!name.startsWith(timelinePrefix) && !name.startsWith(timelineWithFriendsPrefix)
        && !name.startsWith(profilePrefix) && !name.startsWith(repliesPrefix)
        && !name.startsWith(messagesPrefix) && !name.startsWith(userPrefix)) {
        return false;
    }

    TimelineSource::RequestType requestType;

    QString who = name;
    if (name.startsWith(timelineWithFriendsPrefix)) {
        requestType = TimelineSource::TimelineWithFriends;
        who.remove(timelineWithFriendsPrefix);
    } else if (name.startsWith(profilePrefix)) {
        requestType = TimelineSource::Profile;
        who.remove(profilePrefix);
    } else if (name.startsWith(repliesPrefix)) {
        requestType = TimelineSource::Replies;
        who.remove(repliesPrefix);
    } else if (name.startsWith(messagesPrefix)) {
        requestType = TimelineSource::DirectMessages;
        who.remove(messagesPrefix);
    } else if (name.startsWith(userPrefix)) {
        requestType = TimelineSource::User;
        who.remove(userPrefix);
    } else {
        requestType = TimelineSource::Timeline;
        who.remove(timelinePrefix);
    }

    //we want just the service url to index the UserImages source
    //QString m_serviceBaseUrl;
    QStringList account = who.split('@');
    if (account.count() == 2) {
        m_serviceBaseUrl = account.at(1);
    } else {
        m_serviceBaseUrl = "http://twitter.com/";
    }

    ImageSource *imageSource = dynamic_cast<ImageSource*>(containerForSource("UserImages:"+m_serviceBaseUrl));
    connect(imageSource, SIGNAL(dataChanged()), SLOT(imageDataChanged()));

    if (!imageSource) {
        imageSource = new ImageSource(this);
        imageSource->setStorageEnabled(true);

        imageSource->setObjectName("UserImages:"+m_serviceBaseUrl);
        addSource(imageSource);
    }


    if (requestType == TimelineSource::User) {
        UserSource *source = dynamic_cast<UserSource*>(containerForSource(name));

        if (!source) {
            source = new UserSource(account.at(0), m_serviceBaseUrl, this);
            source->setObjectName(name);
            //source->setImageSource(imageSource);
            source->setStorageEnabled(true);

            addSource(source);
        }
        //source->update();

    } else {
        TimelineSource *source = dynamic_cast<TimelineSource*>(containerForSource(name));

        if (!source) {
            source = new TimelineSource(who, requestType, this);
            source->setObjectName(name);
            source->setImageSource(imageSource);
            source->setStorageEnabled(true);

            addSource(source);
        }
        source->update();
    }
    //setData(name, Plasma::DataEngine::Data());

    return false;
}

void TwitterEngine::imageDataChanged()
{
    scheduleSourcesUpdated();
    //const QString s = QString("UserImage:%1").arg(m_serviceBaseUrl);
    //emit DataEngine::dataUpdated(s, ImageSource.data());
}


#include "twitterengine.moc"
