/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ContactsKopete.h"

#include <QDBusServiceWatcher>

#include <KDebug>
#include <KIcon>
#include <KStandardDirs>

#include "kopete_interface.h"
#include "Logger.h"

#define KOPETE_SERVICE "org.kde.kopete"
#define DELAY_INTERVAL 500

namespace Lancelot {
namespace Models {

class ContactsKopete::Private {
public:
    Private(ContactsKopete * parent)
        : interface(NULL), q(parent)
    {}

    ~Private()
    {
    }

    org::kde::Kopete * interface;

    void serviceOnline()
    {
        if (interface) return;

        interface = new org::kde::Kopete(
            KOPETE_SERVICE, "/Kopete", QDBusConnection::sessionBus());
        connect(interface, SIGNAL(contactChanged(QString)),
            q, SLOT(contactChanged(QString)));

        q->load(true);
    }

    void serviceOffline()
    {
        if (!interface) return;

        delete interface;
        interface = NULL;

        q->load(true);
    }

    QDBusServiceWatcher * dbusWatcher;

    QBasicTimer delayTimer;
    QStringList contactsToUpdate;
    bool noOnlineContacts : 1;

private:
    ContactsKopete * const q;
};

ContactsKopete::ContactsKopete()
    : d(new Private(this))
{
    setSelfTitle(i18n("Contacts"));
    setSelfIcon(KIcon("kopete"));

    // service watcher
    d->dbusWatcher = new QDBusServiceWatcher(
        KOPETE_SERVICE, QDBusConnection::sessionBus());

    connect(d->dbusWatcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(kopeteServiceOwnerChanged(QString,QString,QString)));

    // other

    kopeteServiceOwnerChanged(QString::null, QString::null, QString::null);

    load();
}

ContactsKopete::~ContactsKopete()
{
    delete d->interface;
    delete d->dbusWatcher;
    delete d;
}

void ContactsKopete::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->delayTimer.timerId()) {
        d->delayTimer.stop();
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

void ContactsKopete::activate(int index)
{
    if (d->interface && !d->noOnlineContacts) {
        QString data = title(index);
        Logger::self()->log("kopete-model", data);
        d->interface->openChat(data);
    } else {
        BaseModel::activate(index);
    }
    hideApplicationWindow();
}

void ContactsKopete::load()
{
    load(false);
}

void ContactsKopete::load(bool forceReload)
{
    if (forceReload) {
        d->delayTimer.stop();
        d->contactsToUpdate.clear();
    }

    setEmitInhibited(true);

    if (!d->interface) {

        clear();

        if (addService("kopete")) {
            // Item * item = const_cast < Item * > (& itemAt(0));
            // item->title = i18n("Messaging client");
            // item->description = i18n("Messaging client is not running");
        } else {
            add(i18n("Unable to find Kopete"), "",
                    KIcon("application-x-executable"), QVariant("http://kopete.kde.org"));
        }
    } else {
        if (forceReload) {
            // kDebug() << "full";
            clear();
            d->noOnlineContacts = false;

            QDBusReply < QStringList > contacts = d->interface->contactsByFilter("online");
            if (!contacts.isValid()) {
                return;
            }

            foreach (const QString& contact, contacts.value()) {
                 updateContactData(contact);
            }

            if (size() == 0) {
                add(i18n("No online contacts"), "", KIcon("user-offline"), QVariant());
                d->noOnlineContacts = true;
                setSelfTitle(i18n("Contacts"));
            } else {
                setSelfTitle(i18nc("Contacts (number of online contacts)", "Contacts (%1)", QString::number(size())));
            }

        }
    }

    setEmitInhibited(false);
    emit updated();
}

QString ContactsKopete::selfShortTitle() const
{
    if (d->noOnlineContacts) {
        return QString();
    } else {
        return QString::number(size());
    }
}

void ContactsKopete::updateContactData(const QString & contact)
{
    if (d->noOnlineContacts) {
        load(true);
        return;
    }

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
    if (avatarPath.isEmpty()) {
        avatarPath = "user-online";
    }
    kDebug() << "avatarPath" << avatarPath;

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
                d->noOnlineContacts = true;
                add(i18n("No online contacts"), "", KIcon("user-offline"), QVariant());
            }
        }
    }
}

void ContactsKopete::contactChanged(const QString & contactId)
{
    // delaying the update
    if (!d->contactsToUpdate.contains(contactId)) {
        d->contactsToUpdate << contactId;
    }
    d->delayTimer.start(DELAY_INTERVAL, this);
}

void ContactsKopete::kopeteServiceOwnerChanged(const QString & serviceName, const QString & oldOwner, const QString & newOwner)
{
    Q_UNUSED(oldOwner);

    bool enabled;

    if (serviceName.isEmpty()) {
        enabled = QDBusConnection::sessionBus().interface()->isServiceRegistered(KOPETE_SERVICE).value();

    } else {
        enabled = !newOwner.isEmpty();

    }

    kDebug() << "service enabled" << enabled;

    if (enabled) {
        d->serviceOnline();

    } else {
        d->serviceOffline();

    }
}

} // namespace Models
} // namespace Lancelot
