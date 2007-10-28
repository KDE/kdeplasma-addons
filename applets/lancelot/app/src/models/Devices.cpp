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

#include "Devices.h"
#include <KRun>
#include <KDebug>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>

namespace Lancelot {
namespace Models {

Devices::Devices()
{    
    kDebug() << "connecting\n";
    kDebug() << connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this, SLOT(deviceAdded(QString)));
    kDebug() << connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this, SLOT(deviceRemoved(QString)));
    kDebug() << "connected\n";

    loadDevices();
}

Devices::~Devices()
{
}

void Devices::deviceRemoved(const QString & udi)
{
    // TODO: implement 
    kDebug() << "deviceRemoved " << udi << "\n";
    QMutableListIterator<Item> i(m_items);
    int index = 0;
    
    while (i.hasNext()) {
        Item & item = i.next();
        if (item.data.toString() == udi) {
            i.remove();
            // TODO: m_udis.removeAll(udi);
            emit itemDeleted(index);
            return;
        }
        ++index;
    }
    
}

void Devices::deviceAdded(const QString & udi)
{
    kDebug() << "deviceAdded " << udi << "\n";
    addDevice(Solid::Device(udi));
}

void Devices::addDevice(const Solid::Device & device)
{
    const Solid::StorageAccess * access = device.as<Solid::StorageAccess>();
    
    if (!access) return;
    
    connect (
        access, SIGNAL(accessibilityChanged(bool)),
        this, SLOT(udiAccessibilityChanged(bool))
    );
    m_udis[access] = device.udi();

    add(
        device.product(),
        access->filePath(),
        new KIcon(device.icon()),
        device.udi()
    );
}

void Devices::udiAccessibilityChanged(bool accessible)
{
    Solid::StorageAccess * access = (Solid::StorageAccess *) sender();
    // TODO: implement
    if (!m_udis.contains(access)) {
        return;
    }
    QString udi = m_udis[access];
    kDebug() << udi << "\n";
    QMutableListIterator<Item> i(m_items);
    int index = 0;
    
    while (i.hasNext()) {
        Item & item = i.next();
        if (item.data.toString() == udi) {
            break;
        }
        ++index;
    }
    kDebug() << index << "\n";
    
    m_items[index].description = access->filePath();
    emit itemAltered(index);    

}


void Devices::loadDevices()
{
    QList<Solid::Device> deviceList = 
        Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess, QString());

    foreach(const Solid::Device & device, deviceList) {
        addDevice(device);
    }
}

void Devices::freeSpaceInfoAvailable(const QString & mountPoint, quint64 kbSize, quint64 kbUsed, quint64 kbAvailable)
{
    
}

void Devices::activate(int index)
{
    if (index > m_items.size() - 1) return;
    //kDebug() << m_items.at(index).data.toString() << "\n";
    //new KRun(KUrl(m_items.at(index).data.toStringList().at(1)), 0);
    Solid::StorageAccess * access = Solid::Device(m_items.at(index).data.toString()).as<Solid::StorageAccess>();
    
    if (!access) return;

    //if (!access->isAccessible()) {
        access->setup();
    //}
    new KRun(KUrl(access->filePath()), 0);
}

}
}
