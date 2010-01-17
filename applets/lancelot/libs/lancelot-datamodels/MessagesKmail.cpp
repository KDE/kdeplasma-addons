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

#include "MessagesKmail.h"
#include <KIcon>
#include <KRun>
#include <KStandardDirs>

#include "Logger.h"
#include "kmailfolder_interface.h"
#include "kmail_interface.h"

#define CHECK_RUNNING_INTERVAL 5000

namespace Lancelot {
namespace Models {

class MessagesKmail::Private {
public:
    Private()
        : interface(NULL), folderinterface(NULL),
          kmailRunning(false), dataValid(false)
          {}

    ~Private()
    {
        delete interface;
        delete folderinterface;
    }

    org::kde::kmail::kmail * interface;
    org::kde::kmail::folder * folderinterface;
    QBasicTimer timer;
    QString kopeteAvatarsDir;
    bool kmailRunning : 1;
    bool dataValid : 1;
};

MessagesKmail::MessagesKmail()
    : d(new Private())
{
    setSelfTitle(i18n("Unread messages"));
    setSelfIcon(KIcon("kmail"));

    d->interface = new org::kde::kmail::kmail(
            "org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
    d->folderinterface = new org::kde::kmail::folder(
            "org.kde.kmail", "/Folder", QDBusConnection::sessionBus());

    connect(
            d->interface, SIGNAL(unreadCountChanged()),
            this, SLOT(unreadCountChanged())
           );

    d->timer.start(CHECK_RUNNING_INTERVAL, this);
    load();
}

void MessagesKmail::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->timer.timerId()) {
        if (d->kmailRunning != !d->interface->isValid()) {
            d->kmailRunning = d->interface->isValid();
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
    delete d;
}

void MessagesKmail::activate(int index)
{
    if (d->dataValid) {
        QString data = itemAt(index).data.toString();
        Logger::self()->log("kmail-model", data);
        d->interface->selectFolder(data);
    }

    if (d->interface->isValid()) {
        KRun::runCommand("kmail", NULL);
    } else {
        BaseModel::activate(index);
    }
    hideApplicationWindow();
}

void MessagesKmail::load()
{
    setEmitInhibited(true);
    clear();

    if (!d->interface->isValid()) {
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
        d->dataValid = false;
        d->kmailRunning = false;
    } else {
        QDBusReply < QStringList > folders = d->interface->folderList();
        if (!folders.isValid()) {
            d->kmailRunning = false;
            return;
        }
        d->kmailRunning = true;

        foreach (const QString& folder, folders.value()) {
            QDBusReply < QString > rfolder = d->interface->getFolder(folder);
            if (!rfolder.isValid() || rfolder.value().isEmpty()) {
                continue;
            }

            QDBusReply < int > unread = d->folderinterface->unreadMessages();
            if (!unread.isValid() || (unread.value() == 0)) {
                continue;
            }

            QDBusReply < QString > name = d->folderinterface->displayName();
            if (!name.isValid()) {
                continue;
            }

            QDBusReply < QString > path = d->folderinterface->displayPath();
            if (!path.isValid()) {
                continue;
            }

            QDBusReply < QString > ricon = d->folderinterface->normalIconPath();
            KIcon icon;
            add(
                    name.value() + " (" + QString::number(unread) + ')', path,
                    KIcon(
                            (ricon.isValid() && !ricon.value().isEmpty())
                            ? (ricon.value()) : "mail-folder-inbox"
                    ), QVariant(folder)
               );
        }

        d->dataValid = true;

        if (size() == 0) {
            d->dataValid = false;
            add(i18n("No unread mail"), "", KIcon("mail-folder-inbox"), QVariant());
        }

    }
    setEmitInhibited(false);
    emit updated();
}

} // namespace Models
} // namespace Lancelot
