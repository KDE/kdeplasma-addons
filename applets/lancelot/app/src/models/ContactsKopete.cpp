/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "logger/Logger.h"

// #define UPDATE_INTERVAL 15000
#define CHECK_RUNNING_INTERVAL 5000
#define DELAY_INTERVAL 500

namespace Models {

ContactsKopete::ContactsKopete()
    : m_interface(NULL), m_kopeteRunning(false)
{
    setSelfTitle(i18n("Contacts"));
    setSelfIcon(KIcon("kopete"));

    m_interface = new org::kde::Kopete(
            "org.kde.kopete", "/Kopete", QDBusConnection::sessionBus());

    m_kopeteAvatarsDir = KStandardDirs::locate(
            "data", "kopete/avatars/Contacts/");

    m_checkRunningTimer.start(CHECK_RUNNING_INTERVAL, this);
    load();
}

void ContactsKopete::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_checkRunningTimer.timerId()) {
        load();
    } else if (event->timerId() == m_delayTimer.timerId()) {
        qDebug() << "ContactsKopete::contactChanged [delayed]:"
            << m_contactsToUpdate.size();
        m_delayTimer.stop();
        // checking whether we have a large update
        if (m_contactsToUpdate.size() > 5) {
            load(true);
        } else {
            foreach(QString contact, m_contactsToUpdate) {
                updateContactData(contact);
            }
        }
        m_contactsToUpdate.clear();
    }
}

ContactsKopete::~ContactsKopete()
{
    delete m_interface;
}

void ContactsKopete::activate(int index)
{
    if (m_kopeteRunning && !m_noOnlineContacts) {
        QString data = title(index);
        Logger::instance()->log("kopete-model", data);
        m_interface->openChat(data);
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

    if (!m_interface->isValid()) {
        if (m_kopeteRunning) {
            qDebug() << "ContactsKopete::disconnecting D-Bus";
            disconnect(m_interface, SIGNAL(contactChanged(const QString &)),
                    this, SLOT(contactChanged(const QString &)));
            statusChanged = true;
        }

        if (forceReload || statusChanged) {
            clear();
            m_kopeteRunning = false;
            m_checkRunningTimer.start(CHECK_RUNNING_INTERVAL, this);

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
        if (!m_kopeteRunning) {
            qDebug() << "ContactsKopete::connecting D-Bus";
            connect(m_interface, SIGNAL(contactChanged(const QString &)),
                    this, SLOT(contactChanged(const QString &)));
            statusChanged = true;
        }

        if (forceReload || statusChanged) {
            qDebug() << "ContactsKopete::load: full";
            clear();
            m_kopeteRunning = true;
            m_noOnlineContacts = false;

            QDBusReply < QStringList > contacts = m_interface->contactsByFilter("online");
            if (!contacts.isValid()) {
                m_kopeteRunning = false;
                return;
            }

            foreach (const QString& contact, contacts.value()) {
                 updateContactData(contact);
            }

            if (size() == 0) {
                add(i18n("No online contacts"), "", KIcon("user-offline"), QVariant());
                m_noOnlineContacts = true;
            }
        }
    }
    setEmitInhibited(false);
    emit updated();
}

void ContactsKopete::updateContactData(const QString & contact)
{
    // Retrieving contact name
    QDBusReply < QString > contactName = m_interface->getDisplayName(contact);
    if (!contactName.isValid()) {
        return;
    }

    QDBusReply < QVariantMap > contactProperties = m_interface->contactProperties(contact);
    if (!contactProperties.isValid() || contactProperties.value().size() == 0) {
        return;
    }

    QDBusReply < bool > contactOnline = m_interface->isContactOnline(contact);
    if (!contactOnline.isValid()) {
        return;
    }

    QString avatarPath = contactProperties.value().value("picture").toString();
    avatarPath = QUrl(avatarPath).toLocalFile();

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
                m_noOnlineContacts = true;
            }
        }
    }
}

void ContactsKopete::contactChanged(const QString & contactId)
{
    // qDebug() << "ContactsKopete::contactChanged:" << contactId;
    // updateContactData(contactId);
    // delaying the update
    if (!m_contactsToUpdate.contains(contactId)) {
        m_contactsToUpdate << contactId;
    }
    m_delayTimer.start(DELAY_INTERVAL, this);
}

} // namespace Models
