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

#ifndef KRAZYCOLLECTOR_HEADER
#define KRAZYCOLLECTOR_HEADER

#include <QHttpRequestHeader>

#include "icollector.h"
#include "kdeobservatory.h"

class KrazyCollector : public ICollector
{
    Q_OBJECT
public:
    KrazyCollector(const QHash<QString, bool> &krazyReportViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QObject *parent = 0);
    virtual ~KrazyCollector();

    void setLastCollect(QString lastCollect);
    QString lastCollect() const;

    virtual void run();

protected Q_SLOTS:
    virtual void replyFinished(QNetworkReply *reply);

private:
    void collectProject(const QString &project);
    void parseReport(QNetworkReply *reply);
    void processModule(QNetworkReply *reply);

    int m_projectsCollected;
    int m_activeProjects;
    int m_projectsCountDelta;
    QString m_source;
    const QHash<QString, bool> &m_krazyReportViewProjects;
    const QMap<QString, KdeObservatory::Project> &m_projects;
    QMap<QNetworkReply *, QString> m_idFilePrefixMap;
    QMap<QNetworkReply *, QString> m_idProjectNameMap;

    QString m_lastCollect;
};

#endif
