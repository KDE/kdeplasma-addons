/*************************************************************************
 * Copyright 2009 Sandro Andrade sandroandrade@kde.org                   *
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

#include "kdecommitsservice.h"

#include <KUrl>
#include <KIO/Job>
#include <KIO/StoredTransferJob>

#include "kdepresets.h"

KdeCommitsService::KdeCommitsService(KdeCommitsEngine *engine)
: Plasma::Service()
{
    setName("kdecommits");
    m_engine = engine;
}

Plasma::ServiceJob *KdeCommitsService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    if (operation == "allProjectsInfo")
        allProjectsInfo();
    else if (operation == "topActiveProjects")
        topActiveProjects();
    else if (operation == "topProjectDevelopers")
        topProjectDevelopers(parameters["project"].toString());
    else if (operation == "commitHistory")
        commitHistory(parameters["project"].toString());

    return 0;
}

void KdeCommitsService::allProjectsInfo()
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=allProjectsInfo"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeCommitsService::topActiveProjects()
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topActiveProjects&p0=0"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeCommitsService::topProjectDevelopers(const QString &project)
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topProjectDevelopers&p0=" + project + "&p1=0"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeCommitsService::commitHistory(const QString &project)
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=commitHistory&p0=" + project + "&p1=0"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeCommitsService::result(KJob *job)
{
    if (job->error())
    {
        kDebug() << "Job error:" << job->errorText();
    }
    else
    {
        KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>(job);
        
        QString data (storedJob->data());
        QString url = QUrl::fromPercentEncoding(storedJob->url().prettyUrl().toUtf8());
        QString mimeType = storedJob->mimetype();

        if (!data.isEmpty() && mimeType.contains("text/plain"))
        {
            QString project = "";
            QRegExp regexp("\\&p0=(.*)\\&p1");
            if (regexp.indexIn(url, 0) != -1)
                project = regexp.cap(1);
            
            if (url.contains("op=allProjectsInfo"))
            {
                KdePresets::init(data);
                emit engineReady();
            }
            else if (url.contains("op=topActiveProjects"))
            {
                RankValueMap topActiveProjects;
                foreach (QString row, data.split('\n'))
                {
                    if (!row.isEmpty())
                    {
                        QStringList list = row.split(';');
                        QString commits = list.at(1);
                        topActiveProjects.insert(commits.remove('\r').toInt(), list.at(0));
                    }
                }

                m_engine->setData("topActiveProjects", "topActiveProjects", QVariant::fromValue<RankValueMap>(topActiveProjects));
            }
            else if (url.contains("op=topProjectDevelopers"))
            {
                RankValueMap projectTopDevelopers;
                foreach (QString row, data.split('\n'))
                {
                    if (!row.isEmpty())
                    {
                        QStringList list = row.split(';');
                        QString commits = list.at(4);
                        projectTopDevelopers.insert(commits.remove('\r').toInt(), list.at(0));
                    }
                }

                m_engine->setData("topProjectDevelopers", "project", project);
                m_engine->setData("topProjectDevelopers", "topProjectDevelopers", QVariant::fromValue<RankValueMap>(projectTopDevelopers));
            }
            else if (url.contains("op=commitHistory"))
            {
                DateCommitList commitHistory;
                foreach (QString row, data.split('\n'))
                {
                    if (!row.isEmpty())
                    {
                        QStringList list = row.split(';');
                        QString commits = list.at(1);
                        commitHistory.append(QPair<QString, int>(list.at(0), commits.remove('\r').toInt()));
                    }
                }

                m_engine->setData("commitHistory", "project", project);
                m_engine->setData("commitHistory", "commitHistory", QVariant::fromValue<DateCommitList>(commitHistory));
            }
        }
        else
            emit engineError();
    }
}

#include "kdecommitsservice.moc"
