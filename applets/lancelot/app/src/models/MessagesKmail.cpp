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

#include "MessagesKmail.h"
#include <KIcon>
#include <KRun>
#include <KStandardDirs>

#include "logger/Logger.h"

#define CHECK_RUNNING_INTERVAL 5000

namespace Models {

MessagesKmail::MessagesKmail()
    : m_interface(NULL), m_kmailRunning(false), m_dataValid(false)
{
    setSelfTitle(i18n("Unread messages"));
    setSelfIcon(KIcon("kmail"));

    m_interface = new org::kde::kmail::kmail(
            "org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
    m_folderinterface = new org::kde::kmail::folder(
            "org.kde.kmail", "/Folder", QDBusConnection::sessionBus());

    connect(
            m_interface, SIGNAL(unreadCountChanged()),
            this, SLOT(unreadCountChanged())
           );

    m_timer.start(CHECK_RUNNING_INTERVAL, this);
    load();
}

void MessagesKmail::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_timer.timerId()) {
        if (m_kmailRunning != !m_interface->isValid()) {
            m_kmailRunning = m_interface->isValid();
            load();
        }
    }
}

void MessagesKmail::unreadCountChanged()
{
    load();
}

MessagesKmail::~MessagesKmail()
{
    delete m_interface;
    delete m_folderinterface;
}

void MessagesKmail::activate(int index)
{
    if (m_dataValid) {
        QString data = itemAt(index).data.toString();
        Logger::instance()->log("kmail-model", data);
        m_interface->selectFolder(data);
    }

    if (m_interface->isValid()) {
        KRun::runCommand("kmail", NULL);
    } else {
        BaseModel::activate(index);
    }
    hideLancelotWindow();
}

void MessagesKmail::load()
{
    setEmitInhibited(true);
    clear();

    if (!m_interface->isValid()) {
        QStringList services;
        services << "kontact|kmail";

        if (addServices(services) != 0) {
            Item * item = const_cast < Item * > (& itemAt(0));
            item->title = i18n("Mail client");
            item->description = i18n("Mail client is not running");
        } else {
            add(i18n("Unable to find KMail"), "",
                    KIcon("application-x-executable"), QVariant("http://kontact.kde.org"));
        }
        m_dataValid = false;
        m_kmailRunning = false;
    } else {
        QDBusReply < QStringList > folders = m_interface->folderList();
        if (!folders.isValid()) {
            m_kmailRunning = false;
            return;
        }
        m_kmailRunning = true;

        foreach (const QString& folder, folders.value()) {
            QDBusReply < QString > rfolder = m_interface->getFolder(folder);
            if (!rfolder.isValid() || rfolder.value().isEmpty()) {
                continue;
            }

            QDBusReply < int > unread = m_folderinterface->unreadMessages();
            if (!unread.isValid() || (unread.value() == 0)) {
                continue;
            }

            QDBusReply < QString > name = m_folderinterface->displayName();
            if (!name.isValid()) {
                continue;
            }

            QDBusReply < QString > path = m_folderinterface->displayPath();
            if (!path.isValid()) {
                continue;
            }

            QDBusReply < QString > ricon = m_folderinterface->normalIconPath();
            KIcon icon;
            add(
                    name.value() + " (" + QString::number(unread) + ')', path,
                    KIcon(
                            (ricon.isValid() && !ricon.value().isEmpty())
                            ? (ricon.value()) : "mail-folder-inbox"
                    ), QVariant(folder)
               );
        }

        m_dataValid = true;

        if (size() == 0) {
            m_dataValid = false;
            add(i18n("No unread mail"), "", KIcon("mail-folder-inbox"), QVariant());
        }

    }
    setEmitInhibited(false);
    emit updated();
}

} // namespace Models
