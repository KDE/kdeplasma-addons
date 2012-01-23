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

#include <KWebView>
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
        //kDebug() << " user image req'ed, doing nothing: " << name;
        return true;
    }

    if (!name.startsWith(timelinePrefix) && !name.startsWith(timelineWithFriendsPrefix)
        && !name.startsWith(profilePrefix) && !name.startsWith(repliesPrefix)
        && !name.startsWith(messagesPrefix) && !name.startsWith(userPrefix)) {
        return false;
    }

    //kDebug() << "loading: " << name;
    //kDebug() << sources();
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
    //kDebug() << "Updating: " << name;
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
    QString serviceBaseUrl;
    QStringList account = who.split('@');
    if (account.count() == 2) {
        serviceBaseUrl = account.at(1);
    } else {
        serviceBaseUrl = "http://twitter.com/";
    }

    ImageSource *imageSource = dynamic_cast<ImageSource*>(containerForSource("UserImages:"+serviceBaseUrl));

    if (!imageSource) {
        imageSource = new ImageSource(this);
        connect(imageSource, SIGNAL(dataChanged()), SLOT(imageDataChanged()));
        imageSource->setStorageEnabled(true);

        imageSource->setObjectName("UserImages:"+serviceBaseUrl);
        addSource(imageSource);
        //imageSource->loadImage(account.at(0), account.at(1));
    }


    QOAuthHelper *authHelper = 0;
    if (!m_authHelper.contains(serviceBaseUrl)) {
        kDebug() << "Creating new authhelper";
        authHelper = new QOAuthHelper(serviceBaseUrl, this);
        //authHelper->setServiceBaseUrl(serviceBaseUrl);
        m_authHelper[serviceBaseUrl] = authHelper;
        connect(authHelper, SIGNAL(authorizeApp(const QString&, const QString&, const QString&)),
                this, SLOT(authorizeApp(const QString&, const QString&, const QString&)));
        // Run start() here to move to another thread.
        // as we can't share pixmap, this won't work
        // using an invisible webkit
        //authHelper->start();
        authHelper->run();
    } else {
        authHelper = m_authHelper[serviceBaseUrl];
    }

    if (requestType == TimelineSource::User) {
        UserSource *source = dynamic_cast<UserSource*>(containerForSource(name));

        if (!source) {
            source = new UserSource(account.at(0), serviceBaseUrl, this);
            source->setObjectName(name);
            //source->setImageSource(imageSource);
            source->setStorageEnabled(true);
            connect(source, SIGNAL(loadImage(const QString&, const KUrl&)),
                    imageSource, SLOT(loadImage(const QString&, const KUrl&)));
            if (imageSource) {
                imageSource->loadImage(account.at(0), serviceBaseUrl);
            }
            addSource(source);
        }
        source->loadUserInfo(account.at(0), serviceBaseUrl);
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

void TwitterEngine::authorizeApp(const QString &serviceBaseUrl, const QString &authorizeUrl, const QString &pageUrl)
{
    m_webView[serviceBaseUrl] = new KWebView(0);
    m_authorizeUrls << authorizeUrl;
    connect(m_webView[serviceBaseUrl]->page(), SIGNAL(loadFinished(bool)), SLOT(appAuthorized()));

    m_webView[serviceBaseUrl]->page()->mainFrame()->load(pageUrl);

}

void TwitterEngine::appAuthorized()
{
    QWebPage *page = dynamic_cast<QWebPage*>(sender());
    if (!page) {
        kDebug() << "Invalid ..";
        return;
    }
    kDebug() << "Page URL:" << page->mainFrame()->url();
    QString u = page->mainFrame()->url().toString();
    kDebug() << u << " == " << m_authorizeUrls;
    if (m_authorizeUrls.contains(u)) {
        kDebug() << "We're done!";
//         if (d->dialog) {
//             d->dialog->close();
//         }
    } else {
        QString script = "var ackButton = document.getElementById(\"allow\"); ackButton.click();";
        kDebug() << "Script run." << script;
        page->mainFrame()->evaluateJavaScript(script);
    }
    //https://api.twitter.com/oauth/authorize
}


void TwitterEngine::imageDataChanged()
{
    scheduleSourcesUpdated();
    //const QString s = QString("UserImage:%1").arg(serviceBaseUrl);
    //emit DataEngine::dataUpdated(s, ImageSource.data());
}


#include "twitterengine.moc"
