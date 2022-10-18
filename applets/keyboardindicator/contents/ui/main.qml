/*
 *    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *    SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    readonly property QtObject source: PlasmaCore.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: Plasmoid.configuration.key
    }

    function translate(identifier) {
        switch(identifier) {
            // Not using KUIT markup for these newline characters because those
            // get translated into HTML, and this text is displayed in the applet's
            // tooltip which does not render HTML at all for security reasons
            case "Caps Lock": return i18n("Caps Lock activated\n")
            case "Num Lock": return i18n("Num Lock activated\n")
        }
        return identifier;
    }

    function icon(identifier) {
        switch(identifier) {
            case "Num Lock": return "input-num-on"
            case "Caps Lock": return "input-caps-on"
        }
        return null
    }

    readonly property bool lockedCount: {
        var ret = 0;
        for (var v in keystateSource.connectedSources) {
            var data = keystateSource.data[keystateSource.connectedSources[v]];
            ret += data && data.Locked
        }
        return ret
    }

    Plasmoid.icon: {
        for (var v in keystateSource.connectedSources) {
            var source = keystateSource.connectedSources[v]
            var data = keystateSource.data[source];
            if (data && data.Locked)
                return icon(source)
        }
        return "input-caps-on"
    }

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: MouseArea {
        id: compactMouse

        activeFocusOnTab: true
        hoverEnabled: true

        Accessible.name: Plasmoid.title
        Accessible.description: Plasmoid.toolTipSubText
        Accessible.role: Accessible.Button

        property bool wasExpanded: false
        onPressed: wasExpanded = Plasmoid.expanded
        onClicked: Plasmoid.expanded = !wasExpanded

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: Plasmoid.icon
            active: compactMouse.containsMouse
            enabled: root.lockedCount > 0
        }
    }

    Plasmoid.fullRepresentation: PlasmaComponents3.Page {
        implicitWidth: PlasmaCore.Units.gridUnit * 12
        implicitHeight: PlasmaCore.Units.gridUnit * 12

        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (PlasmaCore.Units.gridUnit * 4)
            iconName: Plasmoid.icon
            text: Plasmoid.toolTipSubText
        }
    }

    Plasmoid.status: root.lockedCount > 0 ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.toolTipSubText: {
        var ret = "";
        var found = false;
        for (var v in keystateSource.connectedSources) {
            var source = keystateSource.connectedSources[v]
            var data = keystateSource.data[source];
            if (data && data.Locked) {
                found = true
                ret += translate(source)
            }
        }
        return found ? ret.trim() : i18n("No lock keys activated")
    }
}
