/*
 *   Copyright 2011 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "contenttracker.h"

//#include <KWindowSystem>

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QDBusConnection>

#include "activitymanagerslc_interface.h"

ContentTracker::ContentTracker(QObject *parent)
    : QObject(parent)
{
    connectToActivityManager();
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QStringLiteral("org.kde.ActivityManager"), QDBusConnection::sessionBus(),
                                             QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(serviceChange(QString,QString,QString)));
}

ContentTracker::~ContentTracker()
{
    delete m_activityManagerIface.data();
}

QString ContentTracker::uri() const
{
    return m_uri;
}

QString ContentTracker::mimeType() const
{
    return m_mimetype;
}

QString ContentTracker::title() const
{
    return m_title;
}

void ContentTracker::focusChanged(const QString &uri, const QString &mimetype, const QString &title)
{

#ifndef NDEBUG
    qDebug() << "New URI" << uri << mimetype << title;
#endif

    m_uri = uri;
    m_mimetype = mimetype;
    m_title = title;
    //m_window = (int)KWindowSystem::activeWindow();

    emit changed();
}

void ContentTracker::connectToActivityManager()
{
    delete m_activityManagerIface.data();
    m_activityManagerIface = new OrgKdeActivityManagerSLCInterface(QStringLiteral("org.kde.ActivityManager"), QStringLiteral("/SLC"),
                                    QDBusConnection::sessionBus());
    if (m_activityManagerIface->isValid()) {

        connect(m_activityManagerIface.data(), &OrgKdeActivityManagerSLCInterface::focusChanged,
                this, &ContentTracker::focusChanged);
    } else {
        delete m_activityManagerIface;
        m_activityManagerIface = nullptr;
        qWarning() << "activityManager not reachable";
    }
}

void ContentTracker::serviceChange(const QString& name, const QString& oldOwner, const QString& newOwner)
{
#ifndef NDEBUG
    qDebug()<< "Service" << name << "status change, old owner:" << oldOwner << "new:" << newOwner;
#endif

    if (newOwner.isEmpty()) {
        //unregistered
        delete m_activityManagerIface.data();
    } else if (oldOwner.isEmpty()) {
        //registered
        connectToActivityManager();
    }
}
