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
                QString()
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

    if (data == "switch-user") {
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
