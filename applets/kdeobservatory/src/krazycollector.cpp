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

KrazyCollector::KrazyCollector(const QMap<QString, KdeObservatory::Project> &projects, QObject *parent)
: ICollector(parent),
  m_projects(projects)
{
    m_connectId = setHost("www.englishbreakfastnetwork.org", QHttp::ConnectionModeHttp, 0);
}

KrazyCollector::~KrazyCollector()
{
}

void KrazyCollector::run()
{
    m_projectsCollected = 0;
    KdeObservatoryDatabase::self()->truncateKrazyErrors();
    if (m_projects.count() == 0)
        emit collectFinished();
    else
    {
        QMapIterator<QString, KdeObservatory::Project> i(m_projects);
        while (i.hasNext())
        {
            i.next();
            collectProject(i.key());
        }
    }
}

void KrazyCollector::requestFinished (int id, bool error)
{
    if (error)
        emit collectError(errorString());

    if (id == m_connectId)
        return;

    QString source = readAll();

    QRegExp regExp1("<li><b><u>(.*)</u></b><ol>");
    QRegExp regExp2("<li><span class=\"toolmsg\">(.*)<b>");
    QRegExp regExp3("<li><a href=\"[^<>]*" + m_idProjectMap[id] + "(.*)\">.*</a>:\\s*(.*)\\s*</li>");
    regExp1.setMinimal(true);
    regExp2.setMinimal(true);
    regExp3.setMinimal(true);

    int pos = 0, pos1, pos2, pos3;
    QString fileType;
    QString testName;
    pos1 = regExp1.indexIn(source, pos);
    pos2 = regExp2.indexIn(source, pos);
    pos3 = regExp3.indexIn(source, pos);
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
            KdeObservatoryDatabase::self()->addKrazyError(m_idProjectMap[id], fileType, testName, regExp3.cap(1), regExp3.cap(2));
            pos += regExp3.matchedLength();
        }
        pos1 = regExp1.indexIn(source, pos);
        pos2 = regExp2.indexIn(source, pos);
        pos3 = regExp3.indexIn(source, pos);
    }

    ++m_projectsCollected;
    if (m_projectsCollected == m_projects.count())
        emit collectFinished();
}

void KrazyCollector::collectProject(const QString &project)
{
    int id = get(QUrl::toPercentEncoding("/krazy/reports/" + m_projects[project].krazyReport + "/index.html", "!$&'()*+,;=:@/"));
    m_idProjectMap[id] = m_projects[project].commitSubject.split('/').last();
}
