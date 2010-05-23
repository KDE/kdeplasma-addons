/*************************************************************************
 * Copyright 2010 Sandro Andrade sandroandrade@kde.org                   *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#include "kdeobservatoryengine.h"

#include "kdepresets.h"
#include "kdeobservatoryservice.h"

K_EXPORT_PLASMA_DATAENGINE(kdeobservatory, KdeObservatoryEngine)

KdeObservatoryEngine::KdeObservatoryEngine(QObject *parent, const QVariantList &args)
: Plasma::DataEngine(parent, args)
{
}

void KdeObservatoryEngine::init()
{
    setData("topActiveProjects", "");
    setData("topProjectDevelopers", "");
    setData("commitHistory", "");
    setData("krazyReport", "");
}

bool KdeObservatoryEngine::sourceRequestEvent (const QString &source)
{
    if (source == "allProjectsInfo")
    {
        setData("allProjectsInfo", "views", KdePresets::viewsPreset());
        setData("allProjectsInfo", "viewsActive", KdePresets::viewsActivePreset());
        setData("allProjectsInfo", "automaticallyInViews", KdePresets::automaticallyInViews());
        setData("allProjectsInfo", "projectNames", KdePresets::preset(KdePresets::ProjectName));
        setData("allProjectsInfo", "projectCommitSubjects", KdePresets::preset(KdePresets::ProjectCommitSubject));
        setData("allProjectsInfo", "projectKrazyReports", KdePresets::preset(KdePresets::ProjectKrazyReport));
        setData("allProjectsInfo", "projectKrazyFilePrefixes", KdePresets::preset(KdePresets::ProjectKrazyFilePrefix));
        setData("allProjectsInfo", "projectIcons", KdePresets::preset(KdePresets::ProjectIcon));

        return true;
    }
    return false;
}

Plasma::Service *KdeObservatoryEngine::serviceForSource(const QString &source)
{
    Q_UNUSED(source);
    KdeObservatoryService *service = new KdeObservatoryService(this);
    service->setParent(this);
    return service;
}
