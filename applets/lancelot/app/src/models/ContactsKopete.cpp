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

#define UPDATE_INTERVAL 15000
#define CHECK_RUNNING_INTERVAL 5000

namespace Models {

ContactsKopete::ContactsKopete()
    : m_interface(NULL), m_kopeteRunning(true)
{
    setSelfTitle(i18n("Contacts"));
    setSelfIcon(KIcon("kopete"));

    m_interface = new org::kde::Kopete(
            "org.kde.kopete", "/Kopete", QDBusConnection::sessionBus());

    m_kopeteAvatarsDir = KStandardDirs::locate(
            "data", "kopete/avatars/Contacts/");

    m_timer.start(CHECK_RUNNING_INTERVAL, this);
    load();
}

void ContactsKopete::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_timer.timerId()) {
        load();
    }
}

ContactsKopete::~ContactsKopete()
{
    delete m_interface;
}

void ContactsKopete::activate(int index)
{
    if (m_kopeteRunning) {
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
    setEmitInhibited(true);
    clear();

    if (!m_interface->isValid()) {
            m_kopeteRunning = false;
            m_timer.start(CHECK_RUNNING_INTERVAL, this);

            if (addService("kopete")) {
                Item * item = const_cast < Item * > (& itemAt(0));
                item->title = i18n("Messaging client");
                item->description = i18n("Messaging client is not running");
            } else {
                add(i18n("Unable to find Kopete"), "",
                        KIcon("application-x-executable"), QVariant("http://kopete.kde.org"));
            }
    } else {
        m_kopeteRunning = true;
        m_timer.start(UPDATE_INTERVAL, this);

        QDBusReply < QStringList > contacts = m_interface->contactsByFilter("online");
        if (!contacts.isValid()) {
            m_kopeteRunning = false;
            return;
        }

        foreach (const QString& contact, contacts.value()) {
            // Retrieving contact name
            QDBusReply < QString > contactName = m_interface->getDisplayName(contact);
            if (!contactName.isValid()) {
                continue;
            }

            QDBusReply < QVariantMap > contactProperties = m_interface->contactProperties(contact);
            if (!contactProperties.isValid() || contactProperties.value().size() == 0) {
                continue;
            }

            QString avatarPath = contactProperties.value().value("picture").toString();
            avatarPath = QUrl(avatarPath).toLocalFile();

            QString status = contactProperties.value().value("status_message").toString();
            if (status.isEmpty()) {
                status = contactProperties.value().value("status").toString();
            }

            add(
                contactProperties.value().value("display_name").toString(),
                status,
                KIcon(avatarPath),
                contact);
        }

        if (size() == 0) {
            add(i18n("No online contacts"), "", KIcon("user-offline"), QVariant());
        }
    }
    setEmitInhibited(false);
    emit updated();
}

} // namespace Models
