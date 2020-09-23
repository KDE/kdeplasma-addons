/*
 *    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    readonly property QtObject source: PlasmaCore.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: plasmoid.configuration.key
    }

    function translate(identifier) {
        switch(identifier) {
            case "Caps Lock": return i18n("Caps Lock")
            case "Num Lock": return i18n("Num Lock")
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
        return "input-keyboard"
    }

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.compactRepresentation: PlasmaCore.IconItem {
        source: plasmoid.icon
        active: compactMouse.containsMouse

        MouseArea {
            id: compactMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                plasmoid.expanded = !plasmoid.expanded
            }
        }
    }

    Plasmoid.fullRepresentation: ColumnLayout {
        PlasmaExtras.Heading {
            Layout.fillWidth: true
            level: 3
            wrapMode: Text.WordWrap
            text: root.Plasmoid.toolTipSubText
        }
        Item {
            Layout.fillHeight: true
            width: 5
        }
    }

    Plasmoid.status: lockedCount>0 ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.toolTipSubText: {
        var ret = "";
        var found = false;
        for (var v in keystateSource.connectedSources) {
            var source = keystateSource.connectedSources[v]
            var data = keystateSource.data[source];
            if (data && data.Locked) {
                found = true
                ret+=i18n("%1: Locked\n", translate(source))
            }
        }
        return found ? ret.trim() : i18n("Unlocked")
    }
}
