/***************************************************************************
 *   Copyright 2009 by Eckhart Wörner <ewoerner@kde.org>                   *
 *   Copyright 2010 Frederik Gladhorn <gladhorn@kde.org>                   *
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


#include <Plasma/DataEngine>

#include "activitylist.h"
#include <Plasma/Frame>


K_EXPORT_PLASMA_APPLET(opendesktop, OpenDesktopActivities)


using namespace Plasma;

OpenDesktopActivities::OpenDesktopActivities(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_activityList(0),
      m_engine(0),
      m_updateInterval(10 * 60),
      m_firstUpdateDone(false)
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
        initEngine();
        m_activityList = new ActivityList(m_engine, this);
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
    initEngine();
}

void OpenDesktopActivities::initEngine()
{
    if (m_engine) {
        return;
    }
    m_engine = dataEngine("ocs");
    m_engine->connectSource("Providers", this);
}

void OpenDesktopActivities::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (source == "Providers") {
        foreach(const QString& key, data.keys()) {
            m_engine->connectSource("Activities\\provider:" + key, this);
        }
    } else {
        if (!m_firstUpdateDone) {
            if (data.contains("SourceStatus") && data.value("SourceStatus") == "retrieving") {
                return;
            }
            m_engine->connectSource(source, this, m_updateInterval * 1000);
            m_firstUpdateDone = true;
        }
        
        if (m_activityList) {
            m_activityList->dataUpdated(source, data);
        }
        //kDebug() << "Data: " << data;
    }
}


#include "activities.moc"
