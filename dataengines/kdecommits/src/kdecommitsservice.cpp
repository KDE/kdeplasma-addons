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

#include <QMutexLocker>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "kdepresets.h"

KdeCommitsService::KdeCommitsService(KdeCommitsEngine *engine)
: Plasma::Service()
{
    setName("kdecommits");
    m_engine = engine;
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), SLOT(finished(QNetworkReply *)));
}

Plasma::ServiceJob *KdeCommitsService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    kDebug() << "Starting service operation" << operation;
    
    if (operation == "allProjectsInfo")
        allProjectsInfo();
    else if (operation == "topActiveProjects")
        topActiveProjects();
    else if (operation == "topProjectDevelopers")
        topProjectDevelopers(parameters["project"].toString());

    return 0;
}

void KdeCommitsService::allProjectsInfo()
{
    m_manager->get(QNetworkRequest(QUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=allProjectsInfo")));
}

void KdeCommitsService::topActiveProjects()
{
    kDebug() << "Invocando" << QUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topActiveProjects&p0=0");
    m_manager->get(QNetworkRequest(QUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topActiveProjects&p0=0")));
}

void KdeCommitsService::topProjectDevelopers(const QString &project)
{
    kDebug() << "Invocando" << QUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topProjectDevelopers&p0=" + project + "&p1=0");
    m_manager->get(QNetworkRequest(QUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topProjectDevelopers&p0=" + project + "&p1=0")));    
}

void KdeCommitsService::finished(QNetworkReply *reply)
{
    QString data;
    QString url;
    QString mimeType;
    {
        QMutexLocker locker(&m_replyMutex);
        data = reply->readAll();
        url = reply->url().toString();
        mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    }
    
    kDebug() << url << "retornou" << mimeType;
    
    if (!data.isEmpty() && mimeType.contains("text/plain"))
    {
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
            QRegExp regexp("\\&p0=(.*)\\&p1");
            regexp.indexIn(url, 0);
            QString project = regexp.cap(1);
            
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

            m_engine->setData("topProjectDevelopers", regexp.cap(1), QVariant::fromValue<RankValueMap>(projectTopDevelopers));
        }
    }
    else
        emit engineError();

    reply->deleteLater();
}

#include "kdecommitsservice.moc"
