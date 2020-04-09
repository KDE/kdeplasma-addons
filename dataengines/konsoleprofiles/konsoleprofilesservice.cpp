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

#include "konsoleprofilesservice.h"

#include <QMap>
#include <QDebug>

#include <KNotificationJobUiDelegate>

#include <KIO/CommandLauncherJob>

KonsoleProfilesService::KonsoleProfilesService(QObject* parent, const QString& profileName)
    : Plasma::Service(parent)
{
    setName(QStringLiteral("org.kde.plasma.dataengine.konsoleprofiles"));
    setDestination(profileName);
}

Plasma::ServiceJob* KonsoleProfilesService::createJob(const QString& operation, QMap<QString,QVariant>& parameters)
{
    return new ProfileJob(this, operation, parameters);
}

ProfileJob::ProfileJob(KonsoleProfilesService *service, const QString &operation, const QMap<QString, QVariant> &parameters)
    : Plasma::ServiceJob(service->destination(), operation, parameters, service)
{
}

void ProfileJob::start()
{
    //destination is the profile name, operation is e.g. "open"
 //   QMap<QString, QVariant>jobParameters = parameters();
    const QString operation = operationName();

qDebug() << "SERVICE START...operation: " << operation << " dest: " << destination();
    if (operation == QLatin1String("open")) {
  //      Q_ASSERT(!jobParameters.isEmpty());

        // Would be nice if we could just return this in createJob above
        auto *job = new KIO::CommandLauncherJob(QStringLiteral("konsole"), {
            QStringLiteral("--profile"), destination()
        });
        job->setDesktopName(QStringLiteral("org.kde.konsole"));

        auto *delegate = new KNotificationJobUiDelegate;
        delegate->setAutoErrorHandlingEnabled(true);
        job->setUiDelegate(delegate);

        connect(job, &KIO::CommandLauncherJob::result, this, [this, job] {
            setError(job->error());
            setErrorText(job->errorText());
            emitResult();
        });

        job->start();
    }
}

