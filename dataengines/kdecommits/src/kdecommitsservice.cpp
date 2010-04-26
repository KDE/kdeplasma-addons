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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

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
    if (operation == "allProjectsInfo")
        allProjectsInfo();
    else if (operation == "topActiveProjects")
        topActiveProjects();

    return 0;
}

void KdeCommitsService::allProjectsInfo()
{
    m_manager->get(QNetworkRequest(QUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=allProjectsInfo")));
}

void KdeCommitsService::topActiveProjects()
{
    m_manager->get(QNetworkRequest(QUrl("http://sandroandrade.org/servlets/KdeCommitsServlet?op=topActiveProjects&p0=0")));
}

void KdeCommitsService::finished(QNetworkReply *reply)
{
    QString url = reply->url().toString();
    
    if (url.contains("op=allProjectsInfo"))
    {
        if (reply->header(QNetworkRequest::ContentTypeHeader).toString().contains("text/plain"))
        {
            KdePresets::init(QString(reply->readAll()));
            emit engineReady();
        }
        else
            emit engineError();
    }
    else if (url.contains("op=topActiveProjects"))
    {
        if (reply->header(QNetworkRequest::ContentTypeHeader).toString().contains("text/plain"))
        {
            QMultiMap<int, QString> topActiveProjects;

            QString data (reply->readAll());
            if (!data.isEmpty())
                foreach (QString row, data.split('\n'))
                    if (!row.isEmpty())
                    {
                        QStringList list = row.split(';');
                        QString commits = list.at(1);
                        topActiveProjects.insert(commits.remove('\r').toInt(), list.at(0));
                    }

            m_engine->setData("topActiveProjects", "topActiveProjects", QVariant::fromValue< QMultiMap<int, QString> >(topActiveProjects));
        }
        else
            emit engineError();
    }
    
    reply->deleteLater();
}

#include "kdecommitsservice.moc"
