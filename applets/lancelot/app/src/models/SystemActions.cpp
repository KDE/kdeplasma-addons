/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#include "SystemActions.h"

#include <KRun>
#include <KIcon>
#include <KLocalizedString>
#include <KStandardDirs>
#include "FavoriteApplications.h"
#include <KAuthorized>
#include <KMessageBox>
#include <QDBusInterface>

#include <KJob>
#include <kworkspace/kworkspace.h>
#include <solid/control/powermanager.h>

#include <plasma/abstractrunner.h>
#include "screensaver_interface.h"

#define ID_MENU_LEAVE       "menu-leave"
#define ID_MENU_SWITCH_USER "menu-switch-user"

#define ID_LOCK_SCREEN  "lock-screen"
#define ID_LOGOUT       "leave-logout"
#define ID_REBOOT       "leave-reboot"
#define ID_POWEROFF     "leave-poweroff"
#define ID_SUSPEND_DISK "suspend-disk"
#define ID_SUSPEND_RAM  "suspend-ram"

namespace Models {

SystemActions * SystemActions::m_instance = NULL;

SystemActions::SystemActions()
    : StandardActionTreeModel(NULL),
      delayedActivateItemIndex(-1)
{
}

SystemActions::SystemActions(Item * r)
    : StandardActionTreeModel(r)
{
}

SystemActions::~SystemActions()
{
}

SystemActions * SystemActions::instance()
{
    if (!m_instance) {
        m_instance = new SystemActions();
        m_instance->load();
    }
    return m_instance;
}

Lancelot::ActionTreeModel * SystemActions::action(const QString & id)
{
    QList < Lancelot::ActionTreeModel * > subs;
    subs << this;

    while (!subs.isEmpty()) {
        Lancelot::StandardActionTreeModel * model =
            dynamic_cast < Lancelot::StandardActionTreeModel * >
                (subs.takeFirst());
        if (!model) {
            continue;
        }

        for (int i = 0; i < model->size(); i++) {
            if (id == model->data(i).toString()) {
                if (model->isCategory(i)) {
                    return model->child(i);
                }

                model->activated(i);
                return NULL;
            } else {
                if (model->isCategory(i)) {
                    subs << model->child(i);
                }
            }
        }
    }
    return NULL;
}

QStringList SystemActions::actions() const
{
    QStringList result;
    result << ID_MENU_LEAVE
           << ID_MENU_SWITCH_USER
           << ID_LOCK_SCREEN
           << ID_LOGOUT
           << ID_REBOOT
           << ID_POWEROFF
           << ID_SUSPEND_DISK
           << ID_SUSPEND_RAM;
    return result;
}

QString SystemActions::actionTitle(const QString & id) const
{
    if (id == ID_MENU_LEAVE) {
        return i18n("&Leave");
    } else if (id == ID_MENU_SWITCH_USER) {
        return i18n("Switch &User");
    } else if (id == ID_LOCK_SCREEN) {
        return i18n("Loc&k Session");
    } else if (id == ID_LOGOUT) {
        return i18n("Log &Out");
    } else if (id == ID_REBOOT) {
        return i18n("Re&boot");
    } else if (id == ID_POWEROFF) {
        return i18n("&Shut Down");
    } else if (id == ID_SUSPEND_DISK) {
        return i18n("Suspend to &Disk");
    } else if (id == ID_SUSPEND_RAM) {
        return i18n("Suspend to &RAM");
    }

    return QString();
}

QIcon SystemActions::actionIcon(const QString & id) const
{
    if (id == ID_MENU_LEAVE) {
        return KIcon("system-shutdown");
    } else if (id == ID_MENU_SWITCH_USER) {
        return KIcon("system-switch-user");
    } else if (id == ID_LOCK_SCREEN) {
        return KIcon("system-lock-screen");
    } else if (id == ID_LOGOUT) {
        return KIcon("system-log-out");
    } else if (id == ID_REBOOT) {
        return KIcon("system-reboot");
    } else if (id == ID_POWEROFF) {
        return KIcon("system-shutdown");
    } else if (id == ID_SUSPEND_DISK) {
        return KIcon("system-suspend-hibernate");
    } else if (id == ID_SUSPEND_RAM) {
        return KIcon("system-suspend");
    }

    return QIcon();
}

void SystemActions::load()
{
#define ItemFor( ID ) \
    StandardActionTreeModel::Item ( \
        actionTitle( ID ), QString(), actionIcon( ID ), ID)

    add(new ItemFor(ID_LOCK_SCREEN));

    StandardActionTreeModel::Item * item;

    item = new ItemFor(ID_MENU_LEAVE);
    item->children << new ItemFor(ID_LOGOUT);
    item->children << new ItemFor(ID_REBOOT);
    item->children << new ItemFor(ID_POWEROFF);
    item->children << new ItemFor(ID_SUSPEND_DISK);
    item->children << new ItemFor(ID_SUSPEND_RAM);
    add(item);

    switchUserModel = new Lancelot::ActionTreeModelProxy(
            new Sessions()
            );
    add(new ItemFor(ID_MENU_SWITCH_USER));
#undef ItemFor

    emit updated();
}

Lancelot::StandardActionTreeModel * SystemActions::createChild(int index)
{
    Item * childItem = root()->children.value(index);
    Lancelot::StandardActionTreeModel * model =
            new SystemActions(childItem);
    return model;
}

bool SystemActions::isCategory(int index) const
{
    if (index < 0 || index >= root()->children.size()) {
        return false;
    }

    if (root()->children.at(index)->data.toString()
            == ID_MENU_SWITCH_USER) {
        return true;
    }

    return Lancelot::StandardActionTreeModel::isCategory(index);
}

Lancelot::ActionTreeModel * SystemActions::child(int index)
{
    if (index < 0 || index >= root()->children.size()) {
        return NULL;
    }

    if (root()->children.at(index)->data.toString()
            == ID_MENU_SWITCH_USER) {
        return switchUserModel;
    }

    return Lancelot::StandardActionTreeModel::child(index);
}

void SystemActions::activate(int index)
{
    if (index < 0 || index >= root()->children.size()) {
        delayedActivateItemIndex = -1;
        return;
    }

    delayedActivateItemIndex = index;
    QTimer::singleShot(0, this, SLOT(delayedActivate()));
}

void SystemActions::delayedActivate()
{
    if (delayedActivateItemIndex < 0) {
        return;
    }

    QString cmd = root()->children.at(delayedActivateItemIndex)->data.toString();

    if (cmd == ID_LOCK_SCREEN) {
        org::freedesktop::ScreenSaver screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", QDBusConnection::sessionBus());

        if (screensaver.isValid()) {
            ApplicationConnector::instance()->hide(true);
            screensaver.Lock();
        } else {
            KMessageBox::error(
                    0,
                    i18n("<p>Lancelot can not lock your screen at the moment."
                         "</p>"),
                    i18n("Session locking error"));
        }

        return;
    }

    KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
    KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeDefault; // abuse as "nothing"

    // KWorkSpace related

    if (cmd == ID_LOGOUT) {
        type = KWorkSpace::ShutdownTypeNone;
    } else if (cmd == ID_REBOOT) {
        type = KWorkSpace::ShutdownTypeReboot;
    } else if (cmd == ID_POWEROFF) {
        type = KWorkSpace::ShutdownTypeHalt;
    }

    if (type != KWorkSpace::ShutdownTypeDefault) {
        ApplicationConnector::instance()->hide(true);
        KWorkSpace::requestShutDown(confirm, type);
        return;
    }

    // Solid related

    KJob * job = NULL;
    Solid::Control::PowerManager::SuspendMethod method =
        Solid::Control::PowerManager::UnknownSuspendMethod;

    if (cmd == ID_SUSPEND_DISK) {
        method = Solid::Control::PowerManager::ToDisk;
    } else if (cmd == ID_SUSPEND_RAM) {
        method = Solid::Control::PowerManager::ToRam;
    }

    if (method) {
        ApplicationConnector::instance()->hide(true);
        job = Solid::Control::PowerManager::suspend(method);
        if (job) {
            job->start();
        }
        return;
    }
}

// Sessions
Sessions::Sessions()
    : BaseModel()
{
    load();
}

void Sessions::load()
{
    if (KAuthorized::authorizeKAction("start_new_session") &&
        dm.isSwitchable() &&
        dm.numReserve() >= 0) {
        add(
            i18n("New Session"), "",
            KIcon("system-switch-user"), ID_MENU_SWITCH_USER);
    }

    SessList sessions;
    dm.localSessions(sessions);

    foreach (const SessEnt& session, sessions) {
        if (session.self) {
            continue;
        }

        QString name = KDisplayManager::sess2Str(session);

        add(
            name, QString(),
            KIcon(session.vt ? "utilities-terminal" : "user-identity"),
            name//session.session
        );
    }

    if (size() == 0) {
        add(
                i18n("Display manager error"), QString(),
                KIcon("dialog-warning"),
                QString("display-manager-error")
           );
    }
}

void Sessions::activate(int index)
{
    QString data = itemAt(index).data.toString();

    if (data.isEmpty()) {
        return;
    }

    hideLancelotWindow();

    if (data == "display-manager-error") {
        KMessageBox::error(
                0,
                i18n("<p>Lancelot can not find your display manager. "
                     "This means that it not able to retrieve the list of currently "
                     "running sessions, or start a new one.</p>"),
                i18n("Display manager error"));
    } else if (data == ID_MENU_SWITCH_USER) {
        //TODO: update this message when it changes
        // in sessionrunner
        int result = KMessageBox::warningContinueCancel(
                0,
                i18n("<p>You have chosen to open another desktop session.<br />"
                    "The current session will be hidden "
                    "and a new login screen will be displayed.<br />"
                    "An F-key is assigned to each session; "
                    "F%1 is usually assigned to the first session, "
                    "F%2 to the second session and so on. "
                    "You can switch between sessions by pressing "
                    "Ctrl, Alt and the appropriate F-key at the same time. "
                    "Additionally, the KDE Panel and Desktop menus have "
                    "actions for switching between sessions.</p>",
                    7, 8),
                i18n("Warning - New Session"),
                KGuiItem(i18n("&Start New Session"), "fork"),
                KStandardGuiItem::cancel(),
                ":confirmNewSession",
                KMessageBox::PlainCaption | KMessageBox::Notify);

        if (result == KMessageBox::Cancel) {
            return;
        }

        QDBusInterface screensaver("org.freedesktop.ScreenSaver",
                "/ScreenSaver", "org.freedesktop.ScreenSaver");
        screensaver.call( "Lock" );

        dm.startReserve();
        return;
    }

    SessList sessions;
    if (dm.localSessions(sessions)) {
        foreach (const SessEnt &session, sessions) {
            if (data == KDisplayManager::sess2Str(session)) {
                dm.lockSwitchVT(session.vt);
                break;
            }
        }
    }

    hideLancelotWindow();
}

Sessions::~Sessions()
{
}

} // namespace Models
