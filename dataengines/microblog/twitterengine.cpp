/*
 *   Copyright (C) 2007 Trever Fischer <wm161@wm161.net>
 *   Copyright (C) 2007 André Duffeck <duffeck@kde.org>
 *   Copyright (C) 2007 Chani Armitage <chanika@gmail.com>
 *   Copyright 2012 Sebastian Kügler <sebas@kde.org>
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

#include <KDialog>
#include <KWebView>
#include <QWebElement>
#include <QWebFrame>

#include <KDebug>
#include <KIcon>
#include <ksocketfactory.h>

#include "timelinesource.h"
#include "imagesource.h"
#include "qoauthhelper.h"
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
    setData("Defaults", "UserImage", KIcon ("user-identity").pixmap(48, 48).toImage());
}

TwitterEngine::~TwitterEngine()
{
}

bool TwitterEngine::sourceRequestEvent(const QString &name)
{
    if (name.startsWith("UserImages:")) {
        // these are updated by the engine itself, not consumers
        return true;
    }

    if (!name.startsWith(timelinePrefix) && !name.startsWith(timelineWithFriendsPrefix)
        && !name.startsWith(profilePrefix) && !name.startsWith(repliesPrefix)
        && !name.startsWith(messagesPrefix) && !name.startsWith(userPrefix)) {
        return false;
    }

    scheduleSourcesUpdated();
    updateSourceEvent(name); //start a download
    return true;
}

Plasma::Service* TwitterEngine::serviceForSource(const QString &name)
{
    TimelineSource *source = dynamic_cast<TimelineSource*>(containerForSource(name));
    if (!source) {
        kWarning() << "service for non-timeline source requested." << name << sources();
        return Plasma::DataEngine::serviceForSource(name);
    }

    Plasma::Service *service = source->createService();
    service->setParent(this);
    return service;
}

bool TwitterEngine::updateSourceEvent(const QString &name)
{
    //right now it only makes sense to do an update on timelines
    // FIXME: needed?
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
    QString serviceBaseUrl;
    QStringList account = who.split('@');
    const QString user = account.at(0);

    if (account.count() == 2) {
        serviceBaseUrl = account.at(1);
    } else {
        kWarning() << "service not found. Please request a source such as \"TimelineWithFriends:UserName@ServiceUrl\"";
        serviceBaseUrl = "https://twitter.com/";
        kWarning() << "  Using " << serviceBaseUrl << " instead.";
    }

    ImageSource *imageSource = dynamic_cast<ImageSource*>(containerForSource("UserImages:"+serviceBaseUrl));

    if (!imageSource) {
        imageSource = new ImageSource(this);
        connect(imageSource, SIGNAL(dataChanged()), SLOT(imageDataChanged()));
        imageSource->setStorageEnabled(true);

        imageSource->setObjectName("UserImages:"+serviceBaseUrl);
        addSource(imageSource);
    }

    QOAuthHelper *authHelper = 0;
    if (!m_authHelper.contains(serviceBaseUrl)) {
        authorizationStatusUpdated(serviceBaseUrl, "Idle");
        authHelper = new QOAuthHelper(this);
        authHelper->setUser(user);
        authHelper->setServiceBaseUrl(serviceBaseUrl);
        m_authHelper[serviceBaseUrl] = authHelper;

        connect(authHelper, SIGNAL(accessTokenReceived(QString,QString,QString)),
        this, SLOT(accessTokenReceived(const QString&, const QString&, const QString&)));
        connect(authHelper, SIGNAL(statusUpdated(const QString&, const QString&, const QString&)),
                SLOT(authorizationStatusUpdated(const QString&, const QString&, const QString&)));
        // Run start() here to move to another thread.
        // as we can't share pixmap, this won't work
        // using an invisible webkit
        //authHelper->start();
        authHelper->run();
    } else {
        authHelper = m_authHelper[serviceBaseUrl];

        // FIXME: why oh why is this necessary? It seems authHelper lost the user name here
        // timelines won't be loaded without this, anyway.
        if (!user.isEmpty() && requestType != TimelineSource::User) {
            authHelper->setUser(user);
            authHelper->setServiceBaseUrl(serviceBaseUrl);
            if (authHelper->isAuthorized()) {
                authorizationStatusUpdated(serviceBaseUrl, "Ok");
            }
        }
    }

    if (requestType == TimelineSource::User) {
        UserSource *source = dynamic_cast<UserSource*>(containerForSource(name));

        if (!source && !user.isEmpty()) {
            source = new UserSource(user, serviceBaseUrl, this);
            source->setObjectName(name);
            source->setStorageEnabled(true);
            connect(source, SIGNAL(loadImage(const QString&, const KUrl&)),
                    imageSource, SLOT(loadImage(const QString&, const KUrl&)));
            if (imageSource) {
                imageSource->loadImage(user, serviceBaseUrl);
            }
            addSource(source);
        }
        if (!user.isEmpty()) {
            source->loadUserInfo(user, serviceBaseUrl);
        }
    } else {
        TimelineSource *source = dynamic_cast<TimelineSource*>(containerForSource(name));

        if (!source) {
            if (user.isEmpty()) {
                return false;
            }
            source = new TimelineSource(who, requestType, authHelper,this);
            connect(source, SIGNAL(authorize(const QString&, const QString&, const QString&)),
                   authHelper, SLOT(authorize(const QString&, const QString&, const QString&)));
            source->setObjectName(name);
            source->setImageSource(imageSource);
            source->setStorageEnabled(true);

            addSource(source);
        }
        source->update();
    }
    return false;
}

void TwitterEngine::authorizationStatusUpdated(const QString& serviceBaseUrl, const QString& status, const QString &message)
{
    kDebug() << "Status updated ..." << serviceBaseUrl << status << message;
    setData("Status:" + serviceBaseUrl, "AuthorizationMessage", message);
    setData("Status:" + serviceBaseUrl, "Authorization", status);
    scheduleSourcesUpdated();
}

void TwitterEngine::accessTokenReceived(const QString& serviceBaseUrl, const QString& accessToken, const QString& accessTokenSecret)
{
    Q_UNUSED(accessToken);
    Q_UNUSED(accessTokenSecret);
    authorizationStatusUpdated(serviceBaseUrl, "Ok");
}

void TwitterEngine::imageDataChanged()
{
    //const QString s = QString("UserImage:%1").arg(m_serviceBaseUrl);
    //kDebug() << "im datra chnged: ";
    //emit DataEngine::dataUpdated(s, ImageSource.data());
    scheduleSourcesUpdated();
}


#include "twitterengine.moc"
