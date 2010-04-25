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
}

bool KdeCommitsEngine::sourceRequestEvent(const QString& name)
{
    if (name == "top-projects")
    {
        kDebug() << "top-projects";
        setData("top-projects", "projects", QStringList() << "KDE libs" << "Plasma" << "KDE PIM");
        setData("top-projects", "commits", QStringList() << "30" << "20" << "10");
    }
    else if (name.startsWith("top-developers/"))
    {
        kDebug() << "top-developers do projeto " + name.split('/')[1];
        setData(name, "developers", QStringList() << "Pino" << "Aaron" << "Joao");
        setData(name, "commits", QStringList() << "60" << "50" << "40");
    }
    else if (name.startsWith("commit-history/"))
    {
        kDebug() << "commit-history do projeto " + name.split('/')[1];
        setData(name, "days", QStringList() << "Pino" << "Aaron" << "Joao");
        setData(name, "commits", QStringList() << "60" << "50" << "40");
    }
    else if (name.startsWith("krazy-report/"))
    {
        kDebug() << "krazy-report do projeto " + name.split('/')[1];
        setData(name, "types", QStringList() << "Source Code" << "UI" << "Translation");
        QList<QVariant> list;
        list.append(QStringList() << "Erro 1" << "Erro 2" << "Erro 3");
        list.append(QStringList() << "Erro 4" << "Erro 5" << "Erro 6");
        list.append(QStringList() << "Erro 7" << "Erro 8" << "Erro 9");
        setData(name, "errorLists", list);
    }
    return true;
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

#include "kdecommitsengine.moc"
