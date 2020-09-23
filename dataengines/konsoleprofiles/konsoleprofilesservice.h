/*
 *  SPDX-FileCopyrightText: 2011 Shaun Reich <shaun.reich@kdemail.net>
 *  SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KONSOLEPROFILESSERVICE_H
#define KONSOLEPROFILESSERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>

class KonsoleProfilesService : public Plasma::Service
{
    Q_OBJECT

public:
    KonsoleProfilesService(QObject* parent, const QString& profileName);

protected:
    Plasma::ServiceJob* createJob(const QString& operation, QMap<QString,QVariant>& parameters) override;
};

class ProfileJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    ProfileJob(KonsoleProfilesService *service, const QString& operation, const QMap<QString, QVariant> &parameters);
    void start() override;
};

#endif
