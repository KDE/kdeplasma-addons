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

#include "kdeobservatoryservice.h"

#include <KUrl>
#include <KIO/Job>
#include <KIO/StoredTransferJob>

#include "kdepresets.h"

KdeObservatoryService::KdeObservatoryService(KdeObservatoryEngine *engine)
: Plasma::Service()
{
    setName("kdeobservatory");
    m_engine = engine;
}

Plasma::ServiceJob *KdeObservatoryService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    if (operation == "allProjectsInfo")
        allProjectsInfo();
    else if (operation == "topActiveProjects")
        topActiveProjects();
    else if (operation == "topProjectDevelopers")
        topProjectDevelopers(parameters["project"].toString());
    else if (operation == "commitHistory")
        commitHistory(parameters["project"].toString());
    else if (operation == "krazyReport")
        krazyReport(parameters["project"].toString(), parameters["krazyReport"].toString(), parameters["krazyFilePrefix"].toString());

    return 0;
}

void KdeObservatoryService::allProjectsInfo()
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=allProjectsInfo"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeObservatoryService::topActiveProjects()
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topActiveProjects&p0=0"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeObservatoryService::topProjectDevelopers(const QString &project)
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topProjectDevelopers&p0=" + project + "&p1=0"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeObservatoryService::commitHistory(const QString &project)
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=commitHistory&p0=" + project + "&p1=0"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeObservatoryService::krazyReport(const QString &project, const QString &krazyReport, const QString &krazyFilePrefix)
{
    kDebug() << "Atualizando krazy para projeto" << project << "report:" << krazyReport << "prefix:" << krazyFilePrefix;

//    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=commitHistory&p0=" + project + "&p1=0"), KIO::NoReload, KIO::HideProgressInfo);
//    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void KdeObservatoryService::result(KJob *job)
{
    KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>(job);
    
    QString url = QUrl::fromPercentEncoding(storedJob->url().prettyUrl().toUtf8());
    QString mimeType = storedJob->mimetype();
    QString source = "";
    QRegExp regexp1("\\?op=(.*)(\\&|$)");
    if (regexp1.indexIn(url, 0) != -1)
        source = regexp1.cap(1);
    
    if (job->error())
    {
        emit engineError(source, job->errorText());
    }
    else
    {
        QString data (storedJob->data());

        if (!data.isEmpty() && mimeType.contains("text/plain"))
        {
            QString project = "";
            QRegExp regexp2("\\&p0=(.*)\\&p1");
            if (regexp2.indexIn(url, 0) != -1)
                project = regexp2.cap(1);
            
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
            emit engineError(source, i18n("Empty data or incorrect returned mymetype"));
    }
}

#include "kdeobservatoryservice.moc"
