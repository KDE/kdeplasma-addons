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


#include <KDebug>
#include <KUrl>
#include <KIO/Job>
#include <KIO/StoredTransferJob>

#include "kdepresets.h"

KdeObservatoryService::KdeObservatoryService(KdeObservatoryEngine *engine)
: Plasma::Service()
{
    setName("kdeobservatory");
    m_engine = engine;

    connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(networkStatusChanged(Solid::Networking::Status)));
}

Plasma::ServiceJob *KdeObservatoryService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    if (Solid::Networking::status() != Solid::Networking::Connected && Solid::Networking::status() != Solid::Networking::Unknown)
    {
        emit engineError("fatal", i18n("No active network connection"));
        return 0;
    }
/*
    if (m_engine->m_dataCache.contains(operation) && m_engine->m_dataCache[operation].first == parameters)
    {
        if (operation == "topActiveProjects")
        {
            m_engine->setData("topActiveProjects", "topActiveProjects", m_engine->m_dataCache[operation].second);
        }
        else
        {
            QString project = parameters["project"].toString();
            m_engine->setData(operation, "project", project);
            m_engine->setData(operation, project, m_engine->m_dataCache[operation].second);
        }
        m_engine->forceImmediateUpdateOfAllVisualizations();
        return 0;
    }
*/
    if (operation == "allProjectsInfo")
        allProjectsInfo();
    else if (operation == "topActiveProjects")
        topActiveProjects(parameters);
    else if (operation == "topProjectDevelopers")
        topProjectDevelopers(parameters);
    else if (operation == "commitHistory")
        commitHistory(parameters);
    else if (operation == "krazyReport")
        krazyReport(parameters["project"].toString(), parameters["krazyReport"].toString(), parameters["krazyFilePrefix"].toString());

    return 0;
}

void KdeObservatoryService::allProjectsInfo()
{
    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=allProjectsInfo"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(resultServlet(KJob*)));
}

void KdeObservatoryService::topActiveProjects(QMap<QString, QVariant> &parameters)
{
    QString commitFrom = parameters["commitFrom"].toString();
    QString commitTo   = parameters["commitTo"  ].toString();

    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topActiveProjects&p0=0&p1=" + commitFrom + "&p2=" + commitTo), KIO::NoReload, KIO::HideProgressInfo);
    m_jobParametersMap[job] = parameters;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(resultServlet(KJob*)));
}

void KdeObservatoryService::topProjectDevelopers(QMap<QString, QVariant> &parameters)
{
    QString project    = parameters["project"   ].toString();
    QString commitFrom = parameters["commitFrom"].toString();
    QString commitTo   = parameters["commitTo"  ].toString();

    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topProjectDevelopers&p0=" + project + "&p1=0&p2=" + commitFrom + "&p3=" + commitTo), KIO::NoReload, KIO::HideProgressInfo);
    m_jobParametersMap[job] = parameters;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(resultServlet(KJob*)));
}

void KdeObservatoryService::commitHistory(QMap<QString, QVariant> &parameters)
{
    QString project    = parameters["project"   ].toString();
    QString commitFrom = parameters["commitFrom"].toString();
    QString commitTo   = parameters["commitTo"  ].toString();

    KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=commitHistory&p0=" + project + "&p1=0&p2=" + commitFrom + "&p3=" + commitTo), KIO::NoReload, KIO::HideProgressInfo);
    m_jobParametersMap[job] = parameters;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(resultServlet(KJob*)));
}

void KdeObservatoryService::krazyReport(const QString &project, const QString &krazyReport, const QString &krazyFilePrefix)
{
    KIO::StoredTransferJob *job;

    m_projectKrazyReportMap[project].clear();

    if (krazyReport.contains("reports"))
    {
        m_projectCounterMap[project] = 1;
        job = KIO::storedGet(KUrl("http://www.englishbreakfastnetwork.org/krazy/" + krazyReport), KIO::NoReload, KIO::HideProgressInfo);
    }
    else if (krazyReport.contains("component="))
    {
        m_projectCounterMap[project] = 0;
        job = KIO::storedGet(KUrl("http://www.englishbreakfastnetwork.org/krazy/index.php?" + krazyReport), KIO::NoReload, KIO::HideProgressInfo);
    }

    m_krazyJobMap[job] = QPair<QString, QString>(project, krazyFilePrefix);

    connect(job, SIGNAL(result(KJob*)), this, SLOT(resultEBN(KJob*)));
}

void KdeObservatoryService::networkStatusChanged(Solid::Networking::Status status)
{
    kDebug() << "Network status changed to" << status;
    if (status == Solid::Networking::Connected)
        startOperationCall(operationDescription("allProjectsInfo"));
    else if (status == Solid::Networking::Unconnected)
        emit engineError("fatal", i18n("No active network connection"));
}

void KdeObservatoryService::resultServlet(KJob *job)
{
    KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>(job);

    QString url = QUrl::fromPercentEncoding(storedJob->url().prettyUrl().toUtf8());
    QString mimeType = storedJob->mimetype();

    QString source = "";
    QRegExp regexp1("\\?op=(.*)(\\&|$)");
    regexp1.setMinimal(true);
    if (regexp1.indexIn(url, 0) != -1)
        source = regexp1.cap(1);

    if (job->error())
    {
        emit engineError(source, job->errorString());
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

            if (source == "allProjectsInfo")
            {
                KdePresets::init(data);
                emit engineReady();
            }
            else
            {
                if (source == "topActiveProjects")
                {
                    RankValueMap topActiveProjects;
                    foreach (const QString &row, data.split('\n'))
                    {
                        if (!row.isEmpty())
                        {
                            QStringList list = row.split(';');
                            QString commits = list.at(1);
                            topActiveProjects.insert(commits.remove('\r').toInt(), list.at(0));
                        }
                    }

                    m_engine->setData("topActiveProjects", "topActiveProjects", QVariant::fromValue<RankValueMap>(topActiveProjects));
    //                m_engine->m_dataCache.insert("topActiveProjects", QPair<QMap<QString, QVariant>, QVariant>(m_jobParametersMap[storedJob], QVariant::fromValue<RankValueMap>(topActiveProjects)));
                }
                else if (source == "topProjectDevelopers")
                {
                    RankValueMap projectTopDevelopers;
                    foreach (const QString &row, data.split('\n'))
                    {
                        if (!row.isEmpty())
                        {
                            QStringList list = row.split(';');
                            QString commits = list.at(4);
                            projectTopDevelopers.insert(commits.remove('\r').toInt(), list.at(0));
                        }
                    }

                    m_engine->setData("topProjectDevelopers", "project", project);
                    m_engine->setData("topProjectDevelopers", project, QVariant::fromValue<RankValueMap>(projectTopDevelopers));
    //                m_engine->m_dataCache.insert("topProjectDevelopers", QPair<QMap<QString, QVariant>, QVariant>(m_jobParametersMap[storedJob], QVariant::fromValue<RankValueMap>(projectTopDevelopers)));
                }
                else if (source == "commitHistory")
                {
                    DateCommitList commitHistory;
                    foreach (const QString &row, data.split('\n'))
                    {
                        if (!row.isEmpty())
                        {
                            QStringList list = row.split(';');
                            QString commits = list.at(1);
                            commitHistory.append(QPair<QString, int>(list.at(0), commits.remove('\r').toInt()));
                        }
                    }

                    m_engine->setData("commitHistory", "project", project);
                    m_engine->setData("commitHistory", project, QVariant::fromValue<DateCommitList>(commitHistory));
    //                m_engine->m_dataCache.insert("commitHistory", QPair<QMap<QString, QVariant>, QVariant>(m_jobParametersMap[storedJob], QVariant::fromValue<DateCommitList>(commitHistory)));
                }
                m_engine->setData(source, "appletId", m_jobParametersMap[storedJob]["appletId"]);
                m_engine->forceImmediateUpdateOfAllVisualizations();
                m_jobParametersMap.remove(storedJob);
            }
        }
        else
        {
            if (source == "allProjectsInfo")
                source = "fatal";
            kDebug() << "Error:" << data;
            emit engineError(source, i18n("Empty data or incorrect returned MIME type"));
        }
    }
}

void KdeObservatoryService::resultEBN(KJob *job)
{
    KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>(job);
    
    if (job->error())
    {
        emit engineError("krazyReport", job->errorText());
    }
    else
    {
        QString data (storedJob->data());

        if (!data.isEmpty())
        {
            if (data.contains("<h1>Not Found</h1>"))
            {
                emit engineError("krazyReport", i18n("Krazy report '%1' not found.", QUrl::fromPercentEncoding(storedJob->url().prettyUrl().toUtf8())));
                return;
            }

            if (data.contains("<table style=\"clear: right;\" class=\"sortable\" id=\"reportTable\" cellspacing=\"0\" border=\"0\" width=\"100%\">"))
            {
                processModule(data, storedJob);
            }
            else
            {
                parseReport(data, storedJob);
            }
        }
        else
            emit engineError("krazyReport", i18n("Empty data or incorrect returned MIME type"));
    }
}

void KdeObservatoryService::processModule(const QString &data, KIO::StoredTransferJob *storedJob)
{
    QRegExp exp("<a href=\"(reports.*)\".*<td.*>.*(\\d+).*</td>");
    exp.setMinimal(true);

    QString project = m_krazyJobMap[storedJob].first;
    
    m_projectCounterMap[project] = 0;
    m_projectKrazyReportMap[project].clear();

    int pos = 0;
    while ((pos = exp.indexIn(data, pos)) != -1)
    {
        if (exp.cap(2).toInt() > 0)
        {
            ++m_projectCounterMap[project];

            KIO::StoredTransferJob *job = KIO::storedGet(KUrl("http://www.englishbreakfastnetwork.org/krazy/" + exp.cap(1)), KIO::NoReload, KIO::HideProgressInfo);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(resultEBN(KJob*)));

            m_krazyJobMap[job] = m_krazyJobMap[storedJob];
        }

        pos += exp.matchedLength();
    }

    m_krazyJobMap.remove(storedJob);
}

void KdeObservatoryService::parseReport(const QString &data, KIO::StoredTransferJob *storedJob)
{
    QRegExp regExp1("<li><b><u>(.*)</u></b><ol>");
    QRegExp regExp2("<li><span class=\"toolmsg\">(.*)<b>");
    QRegExp regExp3("<li><a href=\"http://lxr.kde.org/source/[^<>]*" + m_krazyJobMap[storedJob].second + "(.*)\">.*</a>:\\s*(.*)\\s*</li>");
    regExp1.setMinimal(true);
    regExp2.setMinimal(true);
    regExp3.setMinimal(true);

    int pos = 0, pos1, pos2, pos3;
    QString fileType;
    QString testName;
    pos1 = regExp1.indexIn(data, pos);
    pos2 = regExp2.indexIn(data, pos);
    pos3 = regExp3.indexIn(data, pos);
    
    QString project = m_krazyJobMap[storedJob].first;

    while (pos1 != -1 || pos2 != -1 || pos3 != -1)
    {
        pos = pos1;
        if ((pos == -1) || (pos2 != -1 && pos2 < pos))
            pos = pos2;
        if ((pos == -1) || (pos3 != -1 && pos3 < pos))
            pos = pos3;
        if (pos == pos1)
        {
            fileType = regExp1.cap(1);
            pos += regExp1.matchedLength();
        }
        else if (pos == pos2)
        {
            testName = regExp2.cap(1);
            pos += regExp2.matchedLength();
        }
        else if (pos == pos3)
        {
            m_projectKrazyReportMap[project][fileType][testName][regExp3.cap(1)].append(regExp3.cap(2));
            pos += regExp3.matchedLength();
        }
        pos1 = regExp1.indexIn(data, pos);
        pos2 = regExp2.indexIn(data, pos);
        pos3 = regExp3.indexIn(data, pos);
    }

    --m_projectCounterMap[project];
    if (m_projectCounterMap[project] == 0)
    {
        m_engine->setData("krazyReport", "project", project);
        m_engine->setData("krazyReport", project, QVariant::fromValue<KrazyReportMap>(m_projectKrazyReportMap[project]));
        m_engine->forceImmediateUpdateOfAllVisualizations();
        m_projectCounterMap.remove(project);
        m_projectKrazyReportMap.remove(project);
    }

    m_krazyJobMap.remove(storedJob);
}
