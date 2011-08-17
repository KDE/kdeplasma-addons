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

#ifndef KONQPROFILESSERVICE_H
#define KONQPROFILESSERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>

class KonqProfilesService : public Plasma::Service
{
    Q_OBJECT

public:
    KonqProfilesService(QObject* parent, const QString& profileName);

protected:
    Plasma::ServiceJob* createJob(const QString& operation, QMap<QString,QVariant>& parameters);
};

class ProfileJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    ProfileJob(KonqProfilesService *service, const QString& operation, const QMap<QString, QVariant> &parameters);
    void start();
};

#endif
