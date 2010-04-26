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
    connect (m_presetsReply, SIGNAL(finished()), SLOT(presetsReplyFinished()));
    // NOTE: data engine needs to be checked for complete initialization. See KdeObservatory::init().
}

bool KdeCommitsEngine::sourceRequestEvent(const QString& name)
{
    return QMetaObject::invokeMethod(this, QString(name + "Source").toUtf8());
}

bool KdeCommitsEngine::updateSourceEvent(const QString& source)
{
    kDebug() << "Update source " << source;
    return true;
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

    emit sourceReady("presets");
}

void KdeCommitsEngine::presetsReplyFinished()
{
    KdePresets::init(QString(m_presetsReply->readAll()));
    m_presetsReply->deleteLater();
    emit engineReady();
}

void KdeCommitsEngine::topActiveProjectsSource()
{
    kDebug() << "Updating source topActiveProjects";
    m_topActiveProjectsReply = m_commitCollector->runServletOperation("topProjects", QStringList() << "0");
    connect (m_topActiveProjectsReply, SIGNAL(finished()), SLOT(topActiveProjectsReplyFinished()));
}

void KdeCommitsEngine::topActiveProjectsReplyFinished()
{
    QMultiMap<int, QString> topActiveProjects;
    
    QString data (m_topActiveProjectsReply->readAll());
    if (!data.isEmpty())
        foreach (QString row, data.split('\n'))
            if (!row.isEmpty())
            {
                QStringList list = row.split(';');
                QString commits = list.at(1);
                topActiveProjects.insert(commits.remove('\r').toInt(), list.at(0));
            }

    setData("topActiveProjects", QVariant::fromValue< QMultiMap<int, QString> >(topActiveProjects));
    
    m_topActiveProjectsReply->deleteLater();

    emit sourceReady("topActiveProjects");
}

void KdeCommitsEngine::topDevelopersSource()
{
    kDebug() << "Updating source topDevelopers";
//    m_topActiveProjectsReply = m_commitCollector->runServletOperation("topProjects", QStringList() << "0");
//    connect (m_topActiveProjectsReply, SIGNAL(finished()), SLOT(topActiveProjectsReplyFinished()));
    topDevelopersReplyFinished();
}

void KdeCommitsEngine::topDevelopersReplyFinished()
{
    emit sourceReady("topDevelopers");
}

void KdeCommitsEngine::commitHistorySource()
{
    kDebug() << "Updating source commitHistory";
//    m_topActiveProjectsReply = m_commitCollector->runServletOperation("topProjects", QStringList() << "0");
//    connect (m_topActiveProjectsReply, SIGNAL(finished()), SLOT(topActiveProjectsReplyFinished()));
    commitHistoryReplyFinished();
}

void KdeCommitsEngine::commitHistoryReplyFinished()
{
    emit sourceReady("commitHistory");
}

void KdeCommitsEngine::krazyReportSource()
{
    kDebug() << "Updating source krazyReport";
//    m_topActiveProjectsReply = m_commitCollector->runServletOperation("topProjects", QStringList() << "0");
//    connect (m_topActiveProjectsReply, SIGNAL(finished()), SLOT(topActiveProjectsReplyFinished()));
    krazyReportReplyFinished();
}

void KdeCommitsEngine::krazyReportReplyFinished()
{
    emit sourceReady("krazyReport");
}

#include "kdecommitsengine.moc"
