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
#include "koauth.h"
#include "usersource.h"

const QString TwitterEngine::timelinePrefix("Timeline:");
const QString TwitterEngine::timelineWithFriendsPrefix("TimelineWithFriends:");
const QString TwitterEngine::searchTimelinePrefix("SearchTimeline:");
const QString TwitterEngine::customTimelinePrefix("CustomTimeline:");
const QString TwitterEngine::profilePrefix("Profile:");
const QString TwitterEngine::repliesPrefix("Replies:");
const QString TwitterEngine::statusPrefix("Status:");
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
//     kDebug() << name;
    if (name.startsWith("UserImages:")) {
        // these are updated by the engine itself, not consumers
        return true;
    }
    if (name.startsWith(statusPrefix)) {
        kDebug() << "!!!!! Status source : " << name;
        setData(name, "Authorization", "Idle");
        setData(name, "AuthorizationMessage", QString());
        scheduleSourcesUpdated();
        return true;
    }
    if (name == "Accounts") {
        return updateAccounts();
    }
    if (!name.startsWith(timelinePrefix) && !name.startsWith(timelineWithFriendsPrefix)
        && !name.startsWith(customTimelinePrefix) && !name.startsWith(searchTimelinePrefix)
        && !name.startsWith(profilePrefix) && !name.startsWith(repliesPrefix)
        && !name.startsWith(messagesPrefix) && !name.startsWith(userPrefix)
        && !name.startsWith(statusPrefix)) {
        return false;
    }

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
    connect(service, SIGNAL(finished(Plasma::ServiceJob *)), this, SLOT(serviceJobFinished(Plasma::ServiceJob *)));

    return service;
}

void TwitterEngine::serviceJobFinished(Plasma::ServiceJob* job)
{
    kDebug() << "Service Job Done." << job->operationName() << job->result();
}

bool TwitterEngine::updateSourceEvent(const QString &name)
{
    //right now it only makes sense to do an update on timelines
    // FIXME: needed?
    if (!name.startsWith(timelinePrefix) && !name.startsWith(timelineWithFriendsPrefix)
        && !name.startsWith(customTimelinePrefix) && !name.startsWith(searchTimelinePrefix)
        && !name.startsWith(profilePrefix) && !name.startsWith(repliesPrefix)
        && !name.startsWith(messagesPrefix) && !name.startsWith(userPrefix)
        && !name.startsWith(statusPrefix)) {
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
    } else if (name.startsWith(customTimelinePrefix)) {
        requestType = TimelineSource::CustomTimeline;
        who.remove(customTimelinePrefix);
    } else if (name.startsWith(searchTimelinePrefix)) {
        requestType = TimelineSource::SearchTimeline;
        who.remove(searchTimelinePrefix);
        //kDebug() << "Search Timeline requested: " << who;
    } else {
        requestType = TimelineSource::Timeline;
        who.remove(timelinePrefix);
    }
    //we want just the service url to index the UserImages source
    QString serviceBaseUrl;
    QStringList account = who.split('@');
    const QString user = account.at(0);
    QString parameter;
    if (account.count() == 2) {
        QStringList sbu = account.at(1).split(':');
        if (sbu.count() >= 2) {
            serviceBaseUrl = sbu.at(0) + ':' + sbu.at(1); // http + urlpart
            if (sbu.count() > 2) {
                parameter = sbu.at(2);
            }
        }
    } else {
        kWarning() << "service not found. Please request a source such as \"TimelineWithFriends:UserName@ServiceUrl\"";
        serviceBaseUrl = "https://api.twitter.com/1/";
        kWarning() << "  Using " << serviceBaseUrl << " instead.";
    }
    if (name.startsWith(statusPrefix)) {
        authorizationStatusUpdated(user, serviceBaseUrl, "Idle");
        return true;

    }
    ImageSource *imageSource = dynamic_cast<ImageSource*>(containerForSource("UserImages:"+serviceBaseUrl));

    if (!imageSource) {
        imageSource = new ImageSource(this);
        connect(imageSource, SIGNAL(dataChanged()), SLOT(imageDataChanged()));
        imageSource->setStorageEnabled(true);
        imageSource->setObjectName("UserImages:"+serviceBaseUrl);
        addSource(imageSource);
    }

    KOAuth::KOAuth *authHelper = addAuthHelper(user, serviceBaseUrl);

    if (requestType == TimelineSource::User) {
        newUserSource(user, serviceBaseUrl);
    } else {
        TimelineSource *source = dynamic_cast<TimelineSource*>(containerForSource(name));

        if (!source) {
            if (user.isEmpty()) {
                return false;
            }
            source = new TimelineSource(serviceBaseUrl, requestType, authHelper, QStringList() << parameter, this);
            connect(source, SIGNAL(authorize(const QString&, const QString&, const QString&)),
                   authHelper, SLOT(authorize(const QString&, const QString&, const QString&)));
            connect(source, SIGNAL(userFound(const QVariant&, const QString&)),
                   this, SLOT(addUserSource(const QVariant&, const QString&)));
            connect(source, SIGNAL(accountRemoved(const QString&)), SLOT(updateAccounts(const QString&)));
            connect(source, SIGNAL(userData(const QByteArray&)), this, SIGNAL(userData(const QByteArray&)));
            source->setObjectName(name);
            source->setImageSource(imageSource);
            source->setStorageEnabled(true);

            addSource(source);
        }
        source->update();
    }
    return false;
}

bool TwitterEngine::updateAccounts(const QString &removed)
{
    if (sources().contains("Status:" + removed)) {
        setData("Status:" + removed, "Authorization", "Idle");
        setData("Status:" + removed, "AuthorizationMessage", i18n("Account forgotten"));
    }
    if (!sources().contains("Accounts")) {
        //return true;
    }

    foreach (const QString &grp, KOAuth::KOAuth::authorizedAccounts()) {
        if (grp == removed) {
            continue;
        }
        QVariantMap vm;
        QStringList l = grp.split('@');
        if (l.count() < 2) {
            continue;
        }

        const QString user = l[0];
        const QString serviceBaseUrl = l[1];
        vm["accountUser"] = user;
        vm["accountService"] = serviceBaseUrl;
        vm["accountIdentifier"] = grp;
        setData("Accounts", grp, vm);
        QVariantMap m;
        m["screen_name"] = user;

        addAuthHelper(user, serviceBaseUrl);
        newUserSource(user, serviceBaseUrl);
    }
    scheduleSourcesUpdated();
    return true;
}


UserSource* TwitterEngine::newUserSource(const QString userName, const QString serviceBaseUrl)
{
    const QString name = userPrefix+userName+'@'+serviceBaseUrl;
    UserSource *source = dynamic_cast<UserSource*>(containerForSource(name));

    if (!source && !userName.isEmpty()) {
        source = new UserSource(userName, serviceBaseUrl, this);
        source->setObjectName(name);
        source->setStorageEnabled(true);
        connect(this, SIGNAL(userData(const QByteArray&)), source, SLOT(parse(const QByteArray&)));
        ImageSource *imageSource = dynamic_cast<ImageSource*>(containerForSource("UserImages:"+serviceBaseUrl));

        if (!imageSource) {
            imageSource = new ImageSource(this);
            connect(imageSource, SIGNAL(dataChanged()), SLOT(imageDataChanged()));
            imageSource->setStorageEnabled(true);
            imageSource->setObjectName("UserImages:"+serviceBaseUrl);
            addSource(imageSource);
        }

        connect(source, SIGNAL(loadImage(const QString&, const KUrl&)),
                imageSource, SLOT(loadImage(const QString&, const KUrl&)));
        source->loadUserInfo(userName, serviceBaseUrl);

        if (imageSource) {
            imageSource->loadImage(userName);
        }
        addSource(source);
    }
    return source;
}

void TwitterEngine::addUserSource(const QVariant& userData, const QString &serviceBaseUrl)
{
    const QVariantMap m = userData.toMap();
    const QString screen_name = m["screen_name"].toString();
    const QString _s = "User:" + screen_name + "@" + serviceBaseUrl;
    if (sources().contains(_s)) {
        return;
    }

    UserSource *source = new UserSource(screen_name, serviceBaseUrl, this);
    source->setObjectName(_s);
    source->setStorageEnabled(true);
    source->parseJson(userData);
    addSource(source);
}

KOAuth::KOAuth* TwitterEngine::addAuthHelper(const QString& userName, const QString& serviceBaseUrl)
{
    KOAuth::KOAuth *authHelper = 0;
    if (!m_authHelper.contains(serviceBaseUrl)) {
        authorizationStatusUpdated(userName, serviceBaseUrl, "Idle");
        authHelper = new KOAuth::KOAuth(this);
        authHelper->init();
        authHelper->setUser(userName);
        authHelper->setServiceBaseUrl(serviceBaseUrl);
        m_authHelper[serviceBaseUrl] = authHelper;

        connect(authHelper, SIGNAL(accessTokenReceived(const QString&, const QString&, const QString&, const QString&)),
        this, SLOT(accessTokenReceived(const QString&, const QString&, const QString&, const QString&)));
        connect(authHelper, SIGNAL(statusUpdated(const QString&, const QString&, const QString&, const QString&)),
                SLOT(authorizationStatusUpdated(const QString&, const QString&, const QString&, const QString&)));
        // Run start() instead of run() here to move to another thread.
        // as we can't share pixmap, this won't work using an invisible webkit
        //authHelper->start();
        authHelper->run();
        updateSourceEvent(userPrefix + userName + "@" + serviceBaseUrl);
    } else {
        authHelper = m_authHelper[serviceBaseUrl];

        // FIXME: why oh why is this necessary? It seems authHelper lost the user name here
        // timelines won't be loaded without this, anyway.
        if (!userName.isEmpty()) {
            authHelper->setUser(userName);
            authHelper->setServiceBaseUrl(serviceBaseUrl);
            if (authHelper->isAuthorized()) {
                authorizationStatusUpdated(userName, serviceBaseUrl, "Ok");
            }
        }
    }
    return authHelper;
}


void TwitterEngine::authorizationStatusUpdated(const QString &user, const QString& serviceBaseUrl, const QString& status, const QString &message)
{
    const QString src = "Status:" + user + "@" + serviceBaseUrl;
    setData(src, "AuthorizationMessage", message);
    setData(src, "Authorization", status);
    //kDebug() << "Set status to " << status << "for" << src;
    scheduleSourcesUpdated();
}

void TwitterEngine::accessTokenReceived(const QString &user, const QString& serviceBaseUrl, const QString& accessToken, const QString& accessTokenSecret)
{
    Q_UNUSED(accessToken);
    Q_UNUSED(accessTokenSecret);
    authorizationStatusUpdated(user, serviceBaseUrl, "Ok");
}

void TwitterEngine::imageDataChanged()
{
    scheduleSourcesUpdated();
}


#include "twitterengine.moc"
