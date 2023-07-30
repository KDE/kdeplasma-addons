/*
 * SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5

import org.kde.kcmutils // KCMLauncher
import org.kde.config as KConfig  // KAuthorized.authorizeControlModule
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.private.nightcolorcontrol 1.0

PlasmoidItem {
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

    toolTipMainText: i18n("Night Color Control")
    toolTipSubText: {
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

    preferredRepresentation: fullRepresentation

    fullRepresentation: Kirigami.Icon {
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
        KCMLauncher.openSystemSettings("kcm_nightcolor");
    }

    Inhibitor {
        id: inhibitor
    }

    Monitor {
        id: monitor
    }

    PlasmaCore.Action {
        id: configureAction
        text: i18n("&Configure Night Color…")
        icon.name: "configure"
        visible: KConfig.KAuthorized.authorizeControlModule("kcm_nightcolor")
        shortcut: "alt+d, s"
        onTriggered: KCMLauncher.openSystemSettings("kcm_nightcolor")
    }

    Component.onCompleted: {
        Plasmoid.setInternalAction("configure", configureAction)
    }
}
