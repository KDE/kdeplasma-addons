/***************************************************************************
 *   Copyright 2009 by Eckhart Wörner <ewoerner@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "activities.h"

#include <QtCore/QTimer>

#include <KLocale>
#include <KNotification>

#include <Plasma/DataEngine>

#include "activitylist.h"
#include "sourcewatchlist.h"
#include <Plasma/Frame>


K_EXPORT_PLASMA_APPLET(opendesktop, OpenDesktopActivities)


using namespace Plasma;

OpenDesktopActivities::OpenDesktopActivities(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_activityList(0)
{
    KGlobal::locale()->insertCatalog("plasma_applet_opendesktop_activities");
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setPassivePopup(true);

    setPopupIcon("system-users");

    (void)graphicsWidget();
}


QGraphicsWidget* OpenDesktopActivities::graphicsWidget()
{
    if (!m_activityList) {
        m_activityList = new ActivityList(dataEngine("ocs"), this);
        m_activityList->setProvider(m_provider);
        m_activityList->setMinimumSize(300, 300);
    }
    return m_activityList;
}


void OpenDesktopActivities::init()
{
    QTimer::singleShot(0, this, SLOT(initAsync()));
    setAssociatedApplicationUrls(KUrl("http://opendesktop.org"));
}


void OpenDesktopActivities::initAsync()
{
    m_provider = config().readEntry("provider", "https://api.opendesktop.org/v1/");
    if (m_activityList) {
        m_activityList->setProvider(m_provider);
    }

    m_activityWatcher = new SourceWatchList(dataEngine("ocs"), this);
    m_activityWatcher->setQuery("Activities\\provider:" + m_provider);
    connect(m_activityWatcher, SIGNAL(keysAdded(QSet<QString>)), SLOT(newActivities(QSet<QString>)));
}


void OpenDesktopActivities::newActivities(const QSet<QString>& keys)
{
    // FIXME: This still needs to take into account which activities have already been displayed

    // Don't mass-spam the user with activities
    if (keys.size() <= 2) {
        foreach (const QString& key, keys) {
            Plasma::DataEngine::Data activity = m_activityWatcher->value(key).value<Plasma::DataEngine::Data>();
            KNotification* notification = new KNotification("activity");
            notification->setTitle("OpenDesktop Activities");
            notification->setText(activity.value("message").toString());
            notification->setComponentData(KComponentData("plasma-applet-opendesktop-activities", "plasma-applet-opendesktop-activities", KComponentData::SkipMainComponentRegistration));
            notification->sendEvent();
        }
    }
}


#include "activities.moc"
