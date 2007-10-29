/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "SystemServices.h"
#include <KRun>
#include <KLocalizedString>
#include <KDebug>
#include <KServiceTypeTrader>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>

namespace Lancelot {
namespace Models {

SystemServices::SystemServices()
{
    loadSystemServices();
}

SystemServices::~SystemServices()
{
}

void SystemServices::loadSystemServices()
{

    addService("systemsettings");
    addService("ksysguard");
    addService("kinfocenter");
    addService("adept");

}

void SystemServices::addService(const QString & serviceName)
{
    KService::Ptr service = KService::serviceByStorageId(serviceName);
    if (service) {
        QString genericName = service->genericName();
        QString appName = service->name();

        add(
            genericName.isEmpty() ? appName : genericName,
            genericName.isEmpty() ? "" : appName,
            new KIcon(service->icon()),
            service->entryPath() //"settings:/"
        );
    }
}

void SystemServices::activate(int index)
{
    kDebug() << m_items.at(index).data.toString() << "\n";
    new KRun(KUrl(m_items.at(index).data.toString()), 0);
}

}
}
