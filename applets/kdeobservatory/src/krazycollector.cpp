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

#include "krazycollector.h"

#include <QUrl>
#include <QRegExp>

#include "kdeobservatorydatabase.h"

KrazyCollector::KrazyCollector(const QHash<QString, bool> &krazyReportViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QObject *parent)
: ICollector(parent),
  m_header("GET", "/krazy/index.php"),
  m_krazyReportViewProjects(krazyReportViewProjects),
  m_projects(projects)
{
    m_connectId = setHost("www.englishbreakfastnetwork.org", QHttp::ConnectionModeHttp, 0);
    m_header.setValue("Host", "www.englishbreakfastnetwork.org");
    m_header.setValue( "User-Agent", "User Agent");
    m_header.setContentType("application/x-www-form-urlencoded");
}

KrazyCollector::~KrazyCollector()
{
}

void KrazyCollector::run()
{
    m_projectsCollected = 0;
    m_projectsCountDelta = 0;
    m_activeProjects = 0;
    KdeObservatoryDatabase::self()->truncateKrazyErrors();
    bool collected = false;
    QHashIterator<QString, bool> i(m_krazyReportViewProjects);
    QString projectName;
    while (i.hasNext())
    {
        i.next();
        projectName = i.key();
        if (i.value() && !m_projects[projectName].krazyReport.isEmpty())
        {
            collected = true;
            ++m_activeProjects;
            collectProject(projectName);
        }
    }
    if (!collected)
        emit collectFinished();
}

void KrazyCollector::requestFinished (int id, bool error)
{
    if (error)
        emit collectError(errorString());

    if (id == m_connectId)
        return;

    m_source = readAll();

    if (m_source.contains("<h1>Not Found</h1>"))
        kDebug() << "Krazy report not found for:" << m_idProjectNameMap[id] << ". Please, check configuration.";

    if (m_source.contains("<table style=\"clear: right;\" class=\"sortable\" id=\"reportTable\" cellspacing=\"0\" border=\"0\" width=\"100%\">"))
        processModule(id);
    else
        parseReport(id);
}

void KrazyCollector::collectProject(const QString &project)
{
    int id = 0;
    QString krazyReport = m_projects[project].krazyReport;
    if (krazyReport.contains("reports"))
        id = get(QUrl::toPercentEncoding("/krazy/" + krazyReport, "!$&'()*+,;=:@/"));
    else if (krazyReport.contains("component="))
        id = get(QString("/krazy/index.php?" + krazyReport).toUtf8());
    m_idFilePrefixMap[id] = m_projects[project].krazyFilePrefix;
    m_idProjectNameMap[id] = project;
}

void KrazyCollector::parseReport(int id)
{
    QRegExp regExp1("<li><b><u>(.*)</u></b><ol>");
    QRegExp regExp2("<li><span class=\"toolmsg\">(.*)<b>");
    QRegExp regExp3("<li><a href=\"http://lxr.kde.org/source/[^<>]*" + m_idFilePrefixMap[id] + "(.*)\">.*</a>:\\s*(.*)\\s*</li>");
    regExp1.setMinimal(true);
    regExp2.setMinimal(true);
    regExp3.setMinimal(true);

    int pos = 0, pos1, pos2, pos3;
    QString fileType;
    QString testName;
    pos1 = regExp1.indexIn(m_source, pos);
    pos2 = regExp2.indexIn(m_source, pos);
    pos3 = regExp3.indexIn(m_source, pos);
    while (pos1 != -1 || pos2 != -1 || pos3 != -1)
    {
        pos = pos1;
        if (pos2 != -1 && pos2 < pos)
            pos = pos2;
        if (pos3 != -1 && pos3 < pos)
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
            KdeObservatoryDatabase::self()->addKrazyError(m_idProjectNameMap[id], fileType, testName, regExp3.cap(1), regExp3.cap(2));
            pos += regExp3.matchedLength();
        }
        pos1 = regExp1.indexIn(m_source, pos);
        pos2 = regExp2.indexIn(m_source, pos);
        pos3 = regExp3.indexIn(m_source, pos);
    }

    ++m_projectsCollected;
    if (m_projectsCollected == m_activeProjects + m_projectsCountDelta)
        emit collectFinished();
}

void KrazyCollector::processModule(int id)
{
    QRegExp exp("<a href=\"(reports.*)\"");
    exp.setMinimal(true);
    --m_projectsCountDelta;
    int pos = 0;
    while ((pos = exp.indexIn(m_source, pos)) != -1)
    {
        ++m_projectsCountDelta;
        int newId = get(QUrl::toPercentEncoding("/krazy/" + exp.cap(1), "!$&'()*+,;=:@/"));
        m_idFilePrefixMap[newId] = m_idFilePrefixMap[id];
        m_idProjectNameMap[newId] = m_idProjectNameMap[id];
        pos += exp.matchedLength();
    }
}
