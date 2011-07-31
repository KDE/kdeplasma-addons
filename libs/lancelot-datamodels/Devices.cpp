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

#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QDebug>

#include <KMessageBox>
#include <KRun>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KDiskFreeSpaceInfo>
#include <KIcon>
#include <KDebug>

#include "Logger.h"

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/opticaldrive.h>
#include <solid/opticaldisc.h>

#include <cmath>

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
            // setSelfTitle(i18nc("Removable devices", "Removable"));
            setSelfTitle(i18n("Removable devices"));
            setSelfIcon(KIcon("media-optical"));
            break;
        case Models::Devices::Fixed:
            // setSelfTitle(i18nc("Fixed devices", "Fixed"));
            setSelfTitle(i18n("Fixed devices"));
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

    if (!access || udis.contains(device.udi())) return;

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
        access, SIGNAL(accessibilityChanged(bool,QString)),
        this, SLOT(udiAccessibilityChanged(bool,QString))
    );

    QIcon icon = KIcon(device.icon());

    QString description = access->filePath();
    if (!access->isAccessible() || description.isEmpty()) {
        description = i18n("Unmounted");
    } else {
        KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(description);
        if (info.isValid()) {
            qreal percentage = info.used() / (qreal)info.size();

            if (percentage <= 0.999) {
                QPixmap pixmapNormal(32, 32);
                QPixmap pixmapActive(32, 32);

                pixmapNormal.fill(QColor(0, 0, 0, 0));
                pixmapActive.fill(QColor(0, 0, 0, 0));

                QPainter painterNormal(&pixmapNormal);
                QPainter painterActive(&pixmapActive);

                icon.paint(
                    &painterNormal, 0, 0, 24, 24, Qt::AlignCenter, QIcon::Normal);
                icon.paint(
                    &painterActive, 0, 0, 24, 24, Qt::AlignCenter, QIcon::Active);

                painterNormal.setRenderHint(QPainter::Antialiasing);
                painterActive.setRenderHint(QPainter::Antialiasing);

                QColor color;

                color = QColor(0, 0, 0, 100);

                painterNormal.setBrush(color);
                painterNormal.setPen(QPen(color, 0));
                painterNormal.drawEllipse(10, 10, 18, 18);

                painterActive.setBrush(color);
                painterActive.setPen(QPen(color, 0));
                painterActive.drawEllipse(10, 10, 18, 18);

                color = colorForPercentage(percentage);

                painterNormal.setBrush(color);
                color.setAlpha(200);
                painterNormal.setPen(QPen(color.lighter(200), 1));
                painterNormal.drawPie(8, 8, 22, 22,
                    90.0 * 16.0 - 360.0 * 16.0 * percentage,
                    360.0 * 16.0 * percentage);

                painterActive.setBrush(color.lighter());
                color.setAlpha(200);
                painterActive.setPen(QPen(Qt::white, 1));
                painterActive.drawPie(8, 8, 22, 22,
                    90.0 * 16.0 - 360.0 * 16.0 * percentage,
                    360.0 * 16.0 * percentage);

                /*
                 * TODO: Hover - show percentage
                 *
                QFont font = painterActive.font();
                font.setBold(true);
                font.setPixelSize(10);
                painterActive.setFont(font);

                painterActive.setPen(QColor(0, 0, 0));
                painterActive.drawText(12, 22,
                    QString::number(qRound(percentage * 100)) + "%");
                */

                icon = QIcon();
                icon.addPixmap(pixmapNormal);
                icon.addPixmap(pixmapActive, QIcon::Active);
            }
        }
    }

    q->add(
        device.product(),
        description,
        icon,
        device.udi()
    );
}

QColor Devices::Private::combineColors(
        QColor c1, qreal f1,
        QColor c2, qreal f2)
{
    return QColor(
        c1.red()   * f1  + c2.red()   * f2,
        c1.green() * f1  + c2.green() * f2,
        c1.blue()  * f1  + c2.blue()  * f2
    );
}

#define PI 3.1415926535
// Blue 97 147 207
#define LowUsageColor QColor(97, 147, 207)

// Yellow 227 173 0
#define MediumUsageColor QColor(227, 173, 0)

// Red 172 67 17
#define HighUsageColor QColor(172, 67, 17)

QColor Devices::Private::colorForPercentage(qreal percentage)
{
    if (percentage < .25) {
        return LowUsageColor;
    } else if (percentage <= .5) {
        qreal diff = pow(sin((percentage - .25) * 2 * PI), 2);

        return combineColors(
            LowUsageColor,  1 - diff,
            MediumUsageColor, diff
        );
    } else {
        qreal diff = pow(sin(percentage * PI), 2);

        return combineColors(
            MediumUsageColor, diff,
            HighUsageColor, 1 - diff
        );
    }

}

#undef PI

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
        d->addDevice(device);
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
    access->disconnect(this, SLOT(deviceSetupDone(Solid::ErrorType,QVariant,QString)));

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
            connect(access, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)),
                this, SLOT(deviceSetupDone(Solid::ErrorType,QVariant,QString)));
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
