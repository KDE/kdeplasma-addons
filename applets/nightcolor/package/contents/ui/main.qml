/*
 * Copyright 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.5

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.nightcolorcontrol 1.0

Item {
    id: root

    Plasmoid.icon: monitor.running ? 'redshift-status-on' : 'redshift-status-off'

    Plasmoid.status: {
        // Display the applet in the system tray when Night Color is active.
        if (monitor.enabled && monitor.targetTemperature != 6500) {
            return PlasmaCore.Types.ActiveStatus;
        }
        if (inhibitor.state != Inhibitor.Uninhibited) {
            return PlasmaCore.Types.ActiveStatus;
        }
        return PlasmaCore.Types.PassiveStatus;
    }

    Plasmoid.toolTipMainText: i18n("Night Color Control")
    Plasmoid.toolTipSubText: {
        if (inhibitor.state == Inhibitor.Inhibited) {
            return i18n("Night Color is inhibited");
        }
        if (!monitor.available) {
            return i18n("Night Color is unavailable");
        }
        if (!monitor.enabled) {
            return i18n("Night Color is disabled");
        }
        if (!monitor.running) {
            return i18n("Night Color is not running");
        }
        return i18n("Night Color is active (%1K)", monitor.currentTemperature);
    }

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: PlasmaCore.IconItem {
        active: compactMouseArea.containsMouse
        source: plasmoid.icon

        MouseArea {
            id: compactMouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.MiddleButton
            onClicked: toggleInhibition()
        }
    }

    Plasmoid.onActivated: toggleInhibition()

    function toggleInhibition() {
        if (!monitor.available) {
            return;
        }
        switch (inhibitor.state) {
        case Inhibitor.Inhibiting:
        case Inhibitor.Inhibited:
            inhibitor.uninhibit();
            break;
        case Inhibitor.Uninhibiting:
        case Inhibitor.Uninhibited:
            inhibitor.inhibit();
            break;
        }
    }

    function action_KCMNightColor() {
        KCMShell.openSystemSettings("kcm_nightcolor");
    }

    Inhibitor {
        id: inhibitor
    }
    Monitor {
        id: monitor
    }

    Component.onCompleted: {
        if (KCMShell.authorize("kcm_nightcolor.desktop").length > 0) {
            plasmoid.setAction("KCMNightColor", i18n("Configure Night Color..."), "configure");
        }
        plasmoid.removeAction("configure");
    }
}
