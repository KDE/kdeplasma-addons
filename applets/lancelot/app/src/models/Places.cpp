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

#include "Places.h"
#include <KRun>
#include <KLocalizedString>
#include <KDebug>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>

namespace Lancelot {
namespace Models {

Places::Places()
{
    loadPlaces();
}

Places::~Places()
{
}

void Places::loadPlaces()
{
    add(
            i18n("Home Folder"),
            getenv("HOME"),
            new KIcon("user-home"),
            getenv("HOME")
    );
    
    add(
            i18n("Network Folders"),
            "remote:/",
            new KIcon("network"),
            "remote:/"
    );
    
    /*KService::Ptr settingsService = KService::serviceByStorageId("systemsettings");
    if (settingsService) {
        placesItem->appendRow(StandardItemFactory::createItemForService(settingsService));
    }*/

}

void Places::activate(int index)
{
    kDebug() << m_items.at(index).data.toString() << "\n";
    new KRun(KUrl(m_items.at(index).data.toString()), 0);
}

}
}
