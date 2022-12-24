/*
 * SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
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

    Plasmoid.icon: monitor.running ? "redshift-status-on" : "redshift-status-off"

    Plasmoid.status: {
        // Display the applet in the system tray when Night Color is active.
        if (monitor.enabled && monitor.targetTemperature !== 6500) {
            return PlasmaCore.Types.ActiveStatus;
        }
        if (inhibitor.state !== Inhibitor.Uninhibited) {
            return PlasmaCore.Types.ActiveStatus;
        }
        return PlasmaCore.Types.PassiveStatus;
    }

    Plasmoid.toolTipMainText: i18n("Night Color Control")
    Plasmoid.toolTipSubText: {
        if (inhibitor.state === Inhibitor.Inhibited && monitor.enabled) {
            return i18n("Night Color is inhibited");
        }
        if (!monitor.available) {
            return i18n("Night Color is unavailable");
        }
        if (!monitor.enabled) {
            return i18n("Night Color is disabled. Click to configure");
        }
        if (!monitor.running) {
            return i18n("Night Color is not running");
        }
        return i18n("Night Color is active (%1K)", monitor.currentTemperature);
    }

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: PlasmaCore.IconItem {
        active: compactMouseArea.containsMouse
        source: Plasmoid.icon

        MouseArea {
            id: compactMouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.MiddleButton
            onClicked: monitor.enabled ? toggleInhibition() : action_configure()
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

    function action_configure() {
        KCMShell.openSystemSettings("kcm_nightcolor");
    }

    Inhibitor {
        id: inhibitor
    }

    Monitor {
        id: monitor
    }

    Component.onCompleted: {
        Plasmoid.removeAction("configure");
        if (KCMShell.authorize("kcm_nightcolor.desktop").length > 0) {
            Plasmoid.setAction("configure", i18n("&Configure Night Color…"), "configure", "alt+d, s");
        }
    }
}
