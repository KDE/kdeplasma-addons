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

#include <plasma/abstractrunner.h>

namespace Models {

SystemActions * SystemActions::m_instance = NULL;

SystemActions::SystemActions()
    : StandardActionTreeModel(NULL)
{
    kDebug() << "1 created model on" << (void *) m_root;
    kDebug() << "m_instance is" << (void *) m_instance;
}

SystemActions::SystemActions(Item * root)
    : StandardActionTreeModel(root)
{
    kDebug() << "2 creating model on" << (void *) root;
    kDebug() << "2 created model on" << (void *) m_root;
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
    //
    StandardActionTreeModel::Item item(i18n("Leave"), QString(), QIcon(), "leave");
    item.children << Item(i18n("Log Out"), QString(), QIcon(), "log-out");
    item.children << Item(i18n("Reboot"), QString(), QIcon(), "reboot");
    item.children << Item(i18n("Shut Down"), QString(), QIcon(), "poweroff");
    item.children << Item(i18n("Suspend to Disk"), QString(), QIcon(), "suspend-disk");
    item.children << Item(i18n("Suspend to RAM"), QString(), QIcon(), "suspend-ram");
    add(item);

    add(i18n("1"), i18n("1"), QIcon(), "1");
    add(i18n("2"), i18n("1"), QIcon(), "2");
    add(i18n("3"), i18n("1"), QIcon(), "3");
    add(i18n("4"), i18n("1"), QIcon(), "4");
    add(i18n("5"), i18n("1"), QIcon(), "5");
    add(i18n("6"), i18n("1"), QIcon(), "6");
    add(i18n("7"), i18n("1"), QIcon(), "7");
    add(i18n("8"), i18n("1"), QIcon(), "8");
    emit updated();
}

Lancelot::StandardActionTreeModel * SystemActions::createChild(int index)
{
    Item * childItem = & (m_root->children.value(index));
    kDebug() << "Creating child whose index is" << index;
    kDebug() << "Child is" << (void*) childItem << childItem->title;
    Lancelot::StandardActionTreeModel * model =
            new SystemActions(childItem);
    kDebug() << "Confirm" << model->modelTitle();
    return model;
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
    QString data = m_items.at(index).data.toString();
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

        QDBusInterface screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
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
