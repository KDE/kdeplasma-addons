/*
 *    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *    SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasma5support as P5Support
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras

PlasmoidItem {
    id: root

    P5Support.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: Plasmoid.configuration.key
    }

    readonly property list<string> lockedSources: {
        const sources = [];
        for (const source of keystateSource.connectedSources) {
            const data = keystateSource.data[source];
            if (data?.Locked) {
                sources.push(source);
            }
        }
        return sources;
    }

    function translate(identifier: string): string {
        switch (identifier) {
            // Not using KUIT markup for these newline characters because those
            // get translated into HTML, and this text is displayed in the applet's
            // tooltip which does not render HTML at all for security reasons
            case "Caps Lock": return i18n("Caps Lock activated");
            case "Num Lock": return i18n("Num Lock activated");
        }
        return identifier;
    }

    function icon(identifier: string): string {
        switch (identifier) {
            case "Caps Lock": return "input-caps-on";
            case "Num Lock": return "input-num-on";
        }
        return null;
    }

    Plasmoid.icon: {
        if (lockedSources.length > 1) {
            return "input-combo-on";
        } else if (lockedSources.length === 1) {
            return icon(lockedSources[0]);
        } else {
            return "input-caps-on";
        }
    }

    // Only exists because the default CompactRepresentation doesn't expose a
    // way to mark its icon as disabled.
    // TODO remove once it gains that feature.
    compactRepresentation: MouseArea {
        id: compactMouse

        activeFocusOnTab: true
        hoverEnabled: true

        Accessible.name: Plasmoid.title
        Accessible.description: root.toolTipSubText
        Accessible.role: Accessible.Button

        property bool wasExpanded: false
        onPressed: wasExpanded = root.expanded
        onClicked: root.expanded = !wasExpanded

        Kirigami.Icon {
            anchors.fill: parent
            source: Plasmoid.icon
            active: compactMouse.containsMouse
            enabled: root.lockedSources.length > 0
        }
    }

    fullRepresentation: PlasmaComponents.Page {
        implicitWidth: Kirigami.Units.gridUnit * 12
        implicitHeight: Kirigami.Units.gridUnit * 12

        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.gridUnit * 4)
            iconName: Plasmoid.icon
            text: root.toolTipSubText
        }
    }

    Plasmoid.status: lockedSources.length > 0
        ? PlasmaCore.Types.ActiveStatus
        : PlasmaCore.Types.PassiveStatus

    toolTipSubText: {
        if (lockedSources.length > 0) {
            return lockedSources.map(translate).join("\n");
        } else {
            return i18n("No lock keys activated");
        }
    }
}
