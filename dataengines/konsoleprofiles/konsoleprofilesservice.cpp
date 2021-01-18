/*
 *  SPDX-FileCopyrightText: 2011 Shaun Reich <shaun.reich@kdemail.net>
 *  SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "konsoleprofilesservice.h"

#include <QDebug>
#include <QMap>

#include <KNotificationJobUiDelegate>

#include <KIO/CommandLauncherJob>

KonsoleProfilesService::KonsoleProfilesService(QObject *parent, const QString &profileName)
    : Plasma::Service(parent)
{
    setName(QStringLiteral("org.kde.plasma.dataengine.konsoleprofiles"));
    setDestination(profileName);
}

Plasma::ServiceJob *KonsoleProfilesService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    return new ProfileJob(this, operation, parameters);
}

ProfileJob::ProfileJob(KonsoleProfilesService *service, const QString &operation, const QMap<QString, QVariant> &parameters)
    : Plasma::ServiceJob(service->destination(), operation, parameters, service)
{
}

void ProfileJob::start()
{
    // destination is the profile name, operation is e.g. "open"
    //   QMap<QString, QVariant>jobParameters = parameters();
    const QString operation = operationName();

    qDebug() << "SERVICE START...operation: " << operation << " dest: " << destination();
    if (operation == QLatin1String("open")) {
        //      Q_ASSERT(!jobParameters.isEmpty());

        // Would be nice if we could just return this in createJob above
        auto *job = new KIO::CommandLauncherJob(QStringLiteral("konsole"), {QStringLiteral("--profile"), destination()});
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
