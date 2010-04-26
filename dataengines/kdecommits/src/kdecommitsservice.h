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

#ifndef KDECOMMITSSERVICE_HEADER
#define KDECOMMITSSERVICE_HEADER

#include "kdecommitsengine.h"

#include <QMutex>

#include <Plasma/Service>

class QNetworkAccessManager;

namespace Plasma
{
    class ServiceJob;
}

class KdeCommitsService : public Plasma::Service
{
    Q_OBJECT
public:
    KdeCommitsService(KdeCommitsEngine *engine);
    Plasma::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters);

protected:
    void allProjectsInfo();
    void topActiveProjects();
    void topProjectDevelopers(const QString &project);

Q_SIGNALS:
    void engineReady();
    void engineError();

protected slots:
    void finished(QNetworkReply *reply);

private:
    KdeCommitsEngine      *m_engine;
    QNetworkAccessManager *m_manager;
    mutable QMutex         m_replyMutex;
};

typedef QMultiMap<int, QString> RankValueMap;
Q_DECLARE_METATYPE(RankValueMap)

#endif
