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

#include "kdecommitsengine.h"

#include <QNetworkReply>

#include "commitcollector.h"
#include "kdepresets.h"

K_EXPORT_PLASMA_DATAENGINE(kdecommits, KdeCommitsEngine)

KdeCommitsEngine::KdeCommitsEngine(QObject *parent, const QVariantList &args)
: Plasma::DataEngine(parent, args),
  m_commitCollector(new CommitCollector(this))
{
}

void KdeCommitsEngine::init()
{
    m_presetsReply = m_commitCollector->runServletOperation("allProjectsInfo", QStringList());
    connect (m_presetsReply, SIGNAL(finished()), SLOT (presetsReplyFinished()));
    // NOTE: data engine needs to be checked for complete initialization. See KdeObservatory::init()
}

bool KdeCommitsEngine::sourceRequestEvent(const QString& name)
{
    return QMetaObject::invokeMethod(this, QString(name + "Source").toUtf8());
}

bool KdeCommitsEngine::updateSourceEvent(const QString& source)
{
    kDebug() << "Update Source para " << source;
    return true;
}

void KdeCommitsEngine::presetsReplyFinished()
{
    KdePresets::init(QString(m_presetsReply->readAll()));
    m_presetsReply->deleteLater();
}

void KdeCommitsEngine::presetsSource()
{
    setData("presets", "views", KdePresets::viewsPreset());
    setData("presets", "viewsActive", KdePresets::viewsActivePreset());
    setData("presets", "automaticallyInViews", KdePresets::automaticallyInViews());
    setData("presets", "projectNames", KdePresets::preset(KdePresets::ProjectName));
    setData("presets", "projectCommitSubjects", KdePresets::preset(KdePresets::ProjectCommitSubject));
    setData("presets", "projectKrazyReports", KdePresets::preset(KdePresets::ProjectKrazyReport));
    setData("presets", "projectKrazyFilePrefixes", KdePresets::preset(KdePresets::ProjectKrazyFilePrefix));
    setData("presets", "projectIcons", KdePresets::preset(KdePresets::ProjectIcon));
}

#include "kdecommitsengine.moc"
