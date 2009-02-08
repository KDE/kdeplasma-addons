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

#include "Sessions.h"
#include <KRun>
#include <KDebug>
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

namespace Models {

SystemActions * SystemActions::m_instance = NULL;

SystemActions::SystemActions()
    : StandardActionTreeModel(NULL)
{
    kDebug() << "1 created model on" << (void *) root();
    kDebug() << "m_instance is" << (void *) m_instance;
}

SystemActions::SystemActions(Item * r)
    : StandardActionTreeModel(r)
{
    kDebug() << "2 creating model on" << (void *) r;
    kDebug() << "2 created model on" << (void *) root();
}

SystemActions::~SystemActions()
{
}

SystemActions * SystemActions::instance()
{
    kDebug() << (void *) m_instance;
    if (!m_instance) {
        m_instance = new SystemActions();
        m_instance->load();
    }
    kDebug() << (void *) m_instance;
    return m_instance;
}

void SystemActions::load()
{
    add(i18n("Lock Session"), QString(), KIcon("system-lock-screen"), "lock-screen");

    StandardActionTreeModel::Item * item;

    item = new StandardActionTreeModel::Item(i18n("Leave"),
            QString(), KIcon("system-shutdown"), "leave");
    item->children << new Item(i18n("Log Out"),
            QString(), KIcon("system-log-out"), "log-out");
    item->children << new Item(i18n("Reboot"),
            QString(), KIcon("system-restart"), "reboot");
    item->children << new Item(i18n("Shut Down"),
            QString(), KIcon("system-shutdown"), "poweroff");
    item->children << new Item(i18n("Suspend to Disk"),
            QString(), KIcon("system-suspend-hibernate"), "suspend-disk");
    item->children << new Item(i18n("Suspend to RAM"),
            QString(), KIcon("system-suspend"), "suspend-ram");
    add(item);

    switchUserModel = new Lancelot::ActionTreeModelProxy(
            new Sessions()
            );
    add(i18n("Switch User"), QString(), KIcon("system-switch-user"), "switch-user");

    emit updated();
}

Lancelot::StandardActionTreeModel * SystemActions::createChild(int index)
{
    Item * childItem = root()->children.value(index);
    kDebug() << "Creating child whose index is" << index;
    kDebug() << "Child is" << (void*) childItem << childItem->title;
    Lancelot::StandardActionTreeModel * model =
            new SystemActions(childItem);
    kDebug() << "Confirm" << model->modelTitle();
    return model;
}

bool SystemActions::isCategory(int index) const
{
    if (index < 0 || index >= root()->children.size()) {
        return false;
    }

    if (root()->children.at(index)->data.toString()
            == "switch-user") {
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
            == "switch-user") {
        return switchUserModel;
    }

    return Lancelot::StandardActionTreeModel::child(index);
}

void SystemActions::activate(int index)
{
    if (index < 0 || index >= root()->children.size()) {
        return;
    }

    KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
    KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeNone;

    QString cmd = root()->children.at(index)->data.toString();

    // KWorkSpace related

    if (cmd == "log-out") {
        type = KWorkSpace::ShutdownTypeLogout;
    } else if (cmd == "reboot") {
        type = KWorkSpace::ShutdownTypeReboot;
    } else if (cmd == "poweroff") {
        type = KWorkSpace::ShutdownTypeHalt;
    }

    if (type != KWorkSpace::ShutdownTypeNone) {
        ApplicationConnector::instance()->hide(true);
        KWorkSpace::requestShutDown(confirm, type);
        return;
    }

    // Solid related

    KJob * job = NULL;
    Solid::Control::PowerManager::SuspendMethod method =
        Solid::Control::PowerManager::UnknownSuspendMethod;

    if (cmd == "suspend-disk") {
        method = Solid::Control::PowerManager::ToDisk;
    } else if (cmd == "suspend-ram") {
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
            KIcon("system-switch-user"), "switch-user");
    }

    SessList sessions;
    dm.localSessions(sessions);

    foreach (const SessEnt& session, sessions) {
        if (session.self) {
            continue;
        }

        QString name = KDisplayManager::sess2Str(session);

        kDebug() << session.session;
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
    kDebug() << data;

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
    } else if (data == "switch-user") {
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
