/*****************************************************************************
*   Copyright (C) 2011 by Shaun Reich <shaun.reich@kdemail.net>              *
*   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                    *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

#include "konqprofilesservice.h"

#include <QtCore/QMap>
#include <KToolInvocation>
#include <KDebug>

KonqProfilesService::KonqProfilesService(QObject* parent, const QString& profileName)
    : Plasma::Service(parent)
{
    setName("org.kde.plasma.dataengine.konqprofiles");
    setDestination(profileName);
}

Plasma::ServiceJob* KonqProfilesService::createJob(const QString& operation, QMap<QString,QVariant>& parameters)
{
    return new ProfileJob(this, operation, parameters);
}

ProfileJob::ProfileJob(KonqProfilesService *service, const QString &operation, const QMap<QString, QVariant> &parameters)
    : Plasma::ServiceJob(service->destination(), operation, parameters, service)
{
}

void ProfileJob::start()
{
    //destination is the profile name, operation is e.g. "open"
 //   QMap<QString, QVariant>jobParameters = parameters();
    const QString operation = operationName();

kDebug() << "SERVICE START...operation: " << operation << " dest: " << destination();
    if (operation == "open") {
  //      Q_ASSERT(!jobParameters.isEmpty());

        QStringList args;
        args << "--profile" << destination();
        KToolInvocation::kdeinitExec("konsole", args);

        setResult(true);
    }
}

#include "konqprofilesservice.moc"
