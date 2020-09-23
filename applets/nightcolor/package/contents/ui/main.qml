/*
 * SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
