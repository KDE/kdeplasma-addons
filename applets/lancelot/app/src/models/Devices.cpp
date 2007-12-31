/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Devices.h"

#include <QMessageBox>
#include <KRun>
#include <KLocalizedString>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>

namespace Lancelot {
namespace Models {

#define StringCoalesce(A, B) (A == "")?(B):(A)

Devices::Devices(Type filter)
    : m_filter(filter)
{
    load();

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
        this, SLOT(deviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
        this, SLOT(deviceRemoved(QString)));
}

Devices::~Devices()
{
}

void Devices::deviceRemoved(const QString & udi)
{
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
    addDevice(Solid::Device(udi));
}

void Devices::addDevice(const Solid::Device & device)
{
    const Solid::StorageAccess * access = device.as<Solid::StorageAccess>();

    if (!access) return;

    // Testing if it is removable
    if (m_filter != All) {
        bool removable;

        Solid::StorageDrive *drive = 0;
        Solid::Device parentDevice = device;

        while (parentDevice.isValid() && !drive) {
            drive = parentDevice.as<Solid::StorageDrive>();
            parentDevice = parentDevice.parent();
        }

        removable = (drive && (drive->isHotpluggable() || drive->isRemovable()));
        if ((m_filter == Removable) == !removable) return; // Dirty trick simulating XOR
    }

    connect (
        access, SIGNAL(accessibilityChanged(bool, const QString &)),
        this, SLOT(udiAccessibilityChanged(bool, const QString &))
    );
    //m_udis[access] = device.udi();

    add(
        device.product(),
        StringCoalesce(access->filePath(), i18n("Unmounted")),
        new KIcon(device.icon()),
        device.udi()
    );
}

void Devices::udiAccessibilityChanged(bool accessible, const QString & udi)
{
    Q_UNUSED(accessible);

    Solid::StorageAccess * access = Solid::Device(udi).as<Solid::StorageAccess>();
    //if (!m_udis.contains(access)) {
    //    return;
    //}

    QMutableListIterator<Item> i(m_items);
    int index = 0;

    while (i.hasNext()) {
        Item & item = i.next();
        if (item.data.toString() == udi) {
            break;
        }
        ++index;
    }

    m_items[index].description = StringCoalesce(access->filePath(), i18n("Unmounted"));
    emit itemAltered(index);

}


void Devices::load()
{
    QList<Solid::Device> deviceList =
        Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess, QString());

    foreach(const Solid::Device & device, deviceList) {
        addDevice(device);
    }
}

void Devices::freeSpaceInfoAvailable(const QString & mountPoint, quint64 kbSize, quint64 kbUsed, quint64 kbAvailable)
{
    Q_UNUSED(mountPoint);
    Q_UNUSED(kbSize);
    Q_UNUSED(kbUsed);
    Q_UNUSED(kbAvailable);
}

void Devices::activate(int index)
{
    if (index > m_items.size() - 1) return;
    QString udi = m_items.at(index).data.toString();
    Solid::StorageAccess * access = Solid::Device(udi).as<Solid::StorageAccess>();

    if (!access) return;

    if (!access->isAccessible()) {
        //m_devicesMounting << udi;
        connect(access, SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)),
            this, SLOT(deviceSetupDone(Solid::ErrorType, QVariant, const QString &)));
        access->setup();
        return;
    }

    new KRun(KUrl(access->filePath()), 0);
    hideLancelotWindow();
}

void Devices::deviceSetupDone(Solid::ErrorType error, QVariant errorData, const QString & udi)
{
    Q_UNUSED(error);
    Q_UNUSED(errorData);
    //m_devicesMounting.removeAll(udi);

    Solid::StorageAccess * access = Solid::Device(udi).as<Solid::StorageAccess>();
    access->disconnect(this, SLOT(deviceSetupDone(Solid::ErrorType, QVariant, const QString &)));

    if (!access || !access->isAccessible()) {
        QMessageBox::critical(NULL, i18n("Failed to open"), i18n("The requested device can not be accessed."));
        return;
    }

    new KRun(KUrl(access->filePath()), 0);
    hideLancelotWindow();
}


}
}
