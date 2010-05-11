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

#ifndef KDEOBSERVATORYSERVICE_HEADER
#define KDEOBSERVATORYSERVICE_HEADER

#include "kdeobservatoryengine.h"

#include <Plasma/Service>
#include <solid/networking.h>

class KJob;

namespace KIO
{
    class StoredTransferJob;
}

namespace Plasma
{
    class ServiceJob;
}

class KdeObservatoryService : public Plasma::Service
{
    Q_OBJECT
public:
    KdeObservatoryService(KdeObservatoryEngine *engine);
    Plasma::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters);

protected:
    void allProjectsInfo();
    void topActiveProjects(QMap<QString, QVariant> &parameters);
    void topProjectDevelopers(QMap<QString, QVariant> &parameters);
    void commitHistory(QMap<QString, QVariant> &parameters);
    void krazyReport(const QString &project, const QString &krazyReport, const QString &krazyFilePrefix);

Q_SIGNALS:
    void engineReady();
    void engineError(const QString &source, const QString &error);

protected slots:
    void networkStatusChanged(Solid::Networking::Status status);
    void resultServlet(KJob *job);
    void resultEBN(KJob *job);

private:
    void processModule(const QString &data, KIO::StoredTransferJob *storedJob);
    void parseReport(const QString &data, KIO::StoredTransferJob *storedJob);
    
    KdeObservatoryEngine *m_engine;
    QMap< KIO::StoredTransferJob *, QPair<QString, QString> > m_krazyJobMap;
    QMap<QString, KrazyReportMap> m_projectKrazyReportMap;
    QMap<QString, int> m_projectCounterMap;
    QMap< KIO::StoredTransferJob *, QMap<QString, QVariant> > m_jobParametersMap;
};

#endif
