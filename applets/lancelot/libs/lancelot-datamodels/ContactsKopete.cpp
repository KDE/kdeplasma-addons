/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ContactsKopete.h"
#include <KIcon>
#include <KStandardDirs>

#include "kopete_interface.h"
#include "Logger.h"

// #define UPDATE_INTERVAL 15000
#define CHECK_RUNNING_INTERVAL 5000
#define DELAY_INTERVAL 500

namespace Lancelot {
namespace Models {

class ContactsKopete::Private {
public:
    Private()
        : interface(NULL), kopeteRunning(false)
    {}

    ~Private()
    {
        delete interface;
    }

    org::kde::Kopete * interface;
    QBasicTimer delayTimer;
    QBasicTimer checkRunningTimer;
    QStringList contactsToUpdate;
    QString kopeteAvatarsDir;
    bool kopeteRunning : 1;
    bool noOnlineContacts : 1;
};

ContactsKopete::ContactsKopete()
    : d(new Private())
{
    setSelfTitle(i18n("Contacts"));
    setSelfIcon(KIcon("kopete"));

    d->interface = new org::kde::Kopete(
            "org.kde.kopete", "/Kopete", QDBusConnection::sessionBus());

    d->kopeteAvatarsDir = KStandardDirs::locate(
            "data", "kopete/avatars/Contacts/");

    d->checkRunningTimer.start(CHECK_RUNNING_INTERVAL, this);
    load();
}

void ContactsKopete::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->checkRunningTimer.timerId()) {
        load();
    } else if (event->timerId() == d->delayTimer.timerId()) {
        qDebug() << "ContactsKopete::contactChanged [delayed]:"
            << d->contactsToUpdate.size();
        d->delayTimer.stop();
        // checking whether we have a large update
        if (d->contactsToUpdate.size() > 5) {
            load(true);
        } else {
            foreach(const QString &contact, d->contactsToUpdate) {
                updateContactData(contact);
            }
        }
        d->contactsToUpdate.clear();
    }
}

ContactsKopete::~ContactsKopete()
{
    delete d;
}

void ContactsKopete::activate(int index)
{
    if (d->kopeteRunning && !d->noOnlineContacts) {
        QString data = title(index);
        Logger::self()->log("kopete-model", data);
        d->interface->openChat(data);
    } else {
        BaseModel::activate(index);
    }
    hideLancelotWindow();
}

void ContactsKopete::load()
{
    load(false);
}

void ContactsKopete::load(bool forceReload)
{
    setEmitInhibited(true);
    bool statusChanged = false;
    // clear();

    if (!d->interface->isValid()) {
        if (d->kopeteRunning) {
            qDebug() << "ContactsKopete::disconnecting D-Bus";
            disconnect(d->interface, SIGNAL(contactChanged(const QString &)),
                    this, SLOT(contactChanged(const QString &)));
            statusChanged = true;
        }

        if (forceReload || statusChanged) {
            clear();
            d->kopeteRunning = false;
            d->checkRunningTimer.start(CHECK_RUNNING_INTERVAL, this);

            if (addService("kopete")) {
                Item * item = const_cast < Item * > (& itemAt(0));
                item->title = i18n("Messaging client");
                item->description = i18n("Messaging client is not running");
            } else {
                add(i18n("Unable to find Kopete"), "",
                        KIcon("application-x-executable"), QVariant("http://kopete.kde.org"));
            }
        }
    } else {
        if (!d->kopeteRunning) {
            qDebug() << "ContactsKopete::connecting D-Bus";
            connect(d->interface, SIGNAL(contactChanged(const QString &)),
                    this, SLOT(contactChanged(const QString &)));
            statusChanged = true;
        }

        if (forceReload || statusChanged) {
            qDebug() << "ContactsKopete::load: full";
            clear();
            d->kopeteRunning = true;
            d->noOnlineContacts = false;

            QDBusReply < QStringList > contacts = d->interface->contactsByFilter("online");
            if (!contacts.isValid()) {
                d->kopeteRunning = false;
                return;
            }

            foreach (const QString& contact, contacts.value()) {
                 updateContactData(contact);
            }

            if (size() == 0) {
                add(i18n("No online contacts"), "", KIcon("user-offline"), QVariant());
                d->noOnlineContacts = true;
            }
        }
    }
    setEmitInhibited(false);
    emit updated();
}

void ContactsKopete::updateContactData(const QString & contact)
{
    // Retrieving contact name
    QDBusReply < QString > contactName = d->interface->getDisplayName(contact);
    if (!contactName.isValid()) {
        return;
    }

    QDBusReply < QVariantMap > contactProperties = d->interface->contactProperties(contact);
    if (!contactProperties.isValid() || contactProperties.value().size() == 0) {
        return;
    }

    QDBusReply < bool > contactOnline = d->interface->isContactOnline(contact);
    if (!contactOnline.isValid()) {
        return;
    }

    QString avatarPath = contactProperties.value().value("picture").toString();
    avatarPath = KUrl(avatarPath).toLocalFile();

    QString status = contactProperties.value().value("status_message").toString();
    if (status.isEmpty()) {
        status = contactProperties.value().value("status").toString();
    }

    int index;
    for (index = 0; index < size(); index++) {
        if (itemAt(index).data == contact) {
            break;
        }
    }

    if (index >= size()) {
        // we don't have this contact in the list
        if (contactOnline.value()) {
            add(
                contactProperties.value().value("display_name").toString(),
                status,
                KIcon(avatarPath),
                contact);
        }
    } else {
        // we already have this contact
        if (contactOnline.value()) {
            // we are updating the contact
            set(index,
                contactProperties.value().value("display_name").toString(),
                status,
                KIcon(avatarPath),
                contact);
        } else {
            // we are removing the contact from the list
            removeAt(index);

            if (size() == 0) {
                add(i18n("No online contacts"), "", KIcon("user-offline"), QVariant());
                d->noOnlineContacts = true;
            }
        }
    }
}

void ContactsKopete::contactChanged(const QString & contactId)
{
    // qDebug() << "ContactsKopete::contactChanged:" << contactId;
    // updateContactData(contactId);
    // delaying the update
    if (!d->contactsToUpdate.contains(contactId)) {
        d->contactsToUpdate << contactId;
    }
    d->delayTimer.start(DELAY_INTERVAL, this);
}

} // namespace Models
} // namespace Lancelot
