/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include "Devices_p.h"

#include <KMessageBox>
#include <KRun>
#include <KLocalizedString>
#include <KStandardDirs>

#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QDebug>
#include <KIcon>

#include "Logger.h"

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/opticaldrive.h>
#include <solid/opticaldisc.h>

namespace Lancelot {
namespace Models {

Devices::Private::Private(Devices * parent)
    : q(parent)
{
}

Devices::Devices(Type filter)
    : d(new Private(this))
{
    d->filter = filter;

    switch (filter) {
        case Models::Devices::Removable:
            setSelfTitle(i18nc("Removable devices", "Removable"));
            setSelfIcon(KIcon("media-optical"));
            break;
        case Models::Devices::Fixed:
            setSelfTitle(i18nc("Fixed devices", "Fixed"));
            setSelfIcon(KIcon("drive-harddisk"));
            break;
        default:
            setSelfIcon(KIcon("drive-harddisk"));
    }

    load();

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
        d, SLOT(deviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
        d, SLOT(deviceRemoved(QString)));
}

Devices::~Devices()
{
    delete d;
}

void Devices::Private::deviceRemoved(const QString & udi)
{
    for (int i = q->size() - 1; i >= 0; i--) {
        const Item * item = & q->itemAt(i);
        if (item->data.toString() == udi) {
            q->removeAt(i);
            return;
        }
    }
}

void Devices::Private::deviceAdded(const QString & udi)
{
    addDevice(Solid::Device(udi));
}

void Devices::Private::addDevice(const Solid::Device & device)
{
    const Solid::StorageAccess * access = device.as < Solid::StorageAccess > ();

    if (!access) return;

    // Testing if it is removable
    if (filter != All) {
        bool removable;

        Solid::StorageDrive *drive = 0;
        Solid::Device parentDevice = device;

        while (parentDevice.isValid() && !drive) {
            drive = parentDevice.as<Solid::StorageDrive>();
            parentDevice = parentDevice.parent();
        }

        removable = (drive && (drive->isHotpluggable() || drive->isRemovable()));
        if ((filter == Removable) == !removable) return; // Dirty trick simulating XOR
    }

    connect (
        access, SIGNAL(accessibilityChanged(bool, const QString &)),
        this, SLOT(udiAccessibilityChanged(bool, const QString &))
    );

    QString description = access->filePath();
    if (!access->isAccessible() || description.isEmpty()) {
        description = i18n("Unmounted");
    }

    q->add(
        device.product(),
        description,
        KIcon(device.icon()),
        device.udi()
    );
}

void Devices::Private::udiAccessibilityChanged(bool accessible, const QString & udi)
{
    Q_UNUSED(accessible);

    Solid::StorageAccess * access = Solid::Device(udi).as<Solid::StorageAccess>();

    for (int i = q->size() - 1; i >= 0; i--) {
        Item * item = const_cast < Item * > (& q->itemAt(i));
        if (item->data.toString() == udi) {
            item->description = access->filePath();
            if (!access->isAccessible() || item->description.isEmpty()) {
                item->description = i18n("Unmounted");
            }

            emit q->itemAltered(i);
            return;
        }
    }
}

void Devices::load()
{
    QList<Solid::Device> deviceList =
        Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess, QString());

    // Loading hidden UDIs list
    QFile file(
        KStandardDirs::locateLocal("data",
        "kfileplaces/bookmarks.xml"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    d->xmlReader.setDevice(& file);

    while (!d->xmlReader.atEnd()) {
        d->xmlReader.readNext();

        if (d->xmlReader.isStartElement()) {
            if (d->xmlReader.name() == "xbel") {
                d->readXbel();
            }
        }
    }

    // Loading items
    setEmitInhibited(true);
    foreach(const Solid::Device & device, deviceList) {
        if (!d->udis.contains(device.udi())) {
            d->addDevice(device);
        }
    }
    setEmitInhibited(false);
    emit updated();
}

void Devices::Private::readXbel()
{
    while (!xmlReader.atEnd()) {
        xmlReader.readNext();

        if (xmlReader.isEndElement() &&
                xmlReader.name() == "xbel")
            break;

        if (xmlReader.isStartElement()) {
            if (xmlReader.name() == "separator")
                readItem();
        }
    }
}

void Devices::Private::readItem()
{
    QString udi;
    bool showItem = true;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();

        if (xmlReader.isEndElement() && xmlReader.name() == "separator") {
            break;
        }

        if (xmlReader.name() == "UDI") {
            udi = xmlReader.readElementText();
        } else if (xmlReader.name() == "IsHidden") {
            if (xmlReader.readElementText() == "true") {
                showItem = false;
            }
        }
    }

    if (!showItem) {
        udis << udi;
    }
}

void Devices::Private::freeSpaceInfoAvailable(const QString & mountPoint, quint64 kbSize, quint64 kbUsed, quint64 kbAvailable)
{
    Q_UNUSED(mountPoint);
    Q_UNUSED(kbSize);
    Q_UNUSED(kbUsed);
    Q_UNUSED(kbAvailable);
}

void Devices::activate(int index)
{
    if (index > size() - 1) return;

    QString udi = itemAt(index).data.toString();
    Logger::self()->log("devices-model", udi);

    d->setupDevice(udi, true);
}

bool Devices::hasContextActions(int index) const
{
    Q_UNUSED(index);
    return true;
}

void Devices::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    if (index > size() - 1) return;

    QString udi = itemAt(index).data.toString();
    Solid::Device device(udi);
    const Solid::StorageAccess * access = device.as < Solid::StorageAccess > ();

    if (access->filePath().isEmpty() || !access->isAccessible()) {
        menu->addAction(KIcon(device.icon()), i18n("Mount"))
            ->setData(QVariant(1));
    } else if (device.is < Solid::OpticalDisc > ()) {
        menu->addAction(KIcon("media-eject"), i18n("Eject"))
            ->setData(QVariant(0));
    } else {
        menu->addAction(KIcon("media-eject"), i18n("Unmount"))
            ->setData(QVariant(0));
    }
}

void Devices::contextActivate(int index, QAction * context)
{
    if (!context) {
        return;
    }

    QString udi = itemAt(index).data.toString();
    int data = context->data().toInt();

    if (data == 0) {
        d->tearDevice(udi);
    } else if (data == 1) {
        d->setupDevice(udi, false);
    }
}

void Devices::Private::deviceSetupDone(Solid::ErrorType err, QVariant errorData, const QString & udi)
{
    Solid::StorageAccess * access = Solid::Device(udi).as<Solid::StorageAccess>();
    access->disconnect(this, SLOT(deviceSetupDone(Solid::ErrorType, QVariant, const QString &)));

    if (err || !access || !access->isAccessible()) {
        error = errorData.toString();
        QTimer::singleShot(0, this, SLOT(showError()));
        return;
    }

    KRun::runUrl(KUrl(access->filePath()), "inode/directory", 0);
    hideApplicationWindow();
}

void Devices::Private::tearDevice(const QString & udi)
{
    Solid::Device device(udi);
    if (device.is<Solid::OpticalDisc>()) {
        Solid::OpticalDrive *drive = device.parent().as<Solid::OpticalDrive>();
        drive->eject();
    } else {
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

        if (access->isAccessible()) {
            access->teardown();
        }
    }
}

void Devices::Private::setupDevice(const QString & udi, bool openAfterSetup)
{
    Solid::StorageAccess * access = Solid::Device(udi).as < Solid::StorageAccess > ();

    if (!access) return;

    if (access->filePath().isEmpty() || !access->isAccessible()) {
        if (openAfterSetup) {
            connect(access, SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)),
                this, SLOT(deviceSetupDone(Solid::ErrorType, QVariant, const QString &)));
        }
        access->setup();
        return;
    } else if (openAfterSetup) {
        KRun::runUrl(KUrl(access->filePath()), "inode/directory", 0);
        hideApplicationWindow();
    }
}

void Devices::Private::showError()
{
    KMessageBox::detailedError(NULL, i18n("The requested device can not be accessed."), error, i18n("Failed to open"));
}

QMimeData * Devices::mimeData(int index) const
{
    QString udi = itemAt(index).data.toString();

    Solid::StorageAccess * access = Solid::Device(udi).as < Solid::StorageAccess > ();

    if (!access) return NULL;

    if (access->filePath().isEmpty() || !access->isAccessible()) {
        return NULL;
    } else {
        return BaseModel::mimeForUrl(access->filePath());
    }
}

} // namespace Models
} // namespace Lancelot
