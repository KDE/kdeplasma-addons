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
import org.kde.plasma.private.keyboardindicator as KeyboardIndicator
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras

PlasmoidItem {
    id: root

    readonly property KeyboardIndicator.KeyState capsLockState: Plasmoid.configuration.key.includes("Caps Lock") ? kiComponent.createObject(null, {key: Qt.Key_CapsLock}) : null
    readonly property KeyboardIndicator.KeyState numLockState: Plasmoid.configuration.key.includes("Num Lock") ? kiComponent.createObject(null, {key: Qt.Key_NumLock}) : null

    readonly property KeyboardIndicator.KeyState altState: kiComponent.createObject(null, {key: Qt.Key_Alt})
    readonly property KeyboardIndicator.KeyState shiftState: kiComponent.createObject(null, {key: Qt.Key_Shift})
    readonly property KeyboardIndicator.KeyState controlState: kiComponent.createObject(null, {key: Qt.Key_Control})
    readonly property KeyboardIndicator.KeyState metaState: kiComponent.createObject(null, {key: Qt.Key_Meta})
    readonly property KeyboardIndicator.KeyState altGrState: kiComponent.createObject(null, {key: Qt.Key_AltGr})

    readonly property bool anyActive: {
        var ret = anyModifierActive

        if (capsLockState) {
            ret = ret || capsLockState.locked
        }

        if (numLockState) {
            ret = ret || numLockState.locked
        }
        return ret
    }

    readonly property bool anyModifierActive: altState.latched || altState.locked || shiftState.latched || shiftState.locked || controlState.latched || controlState.locked || metaState.latched || metaState.locked || altGrState.latched || altGrState.locked

    Plasmoid.icon: {

        if (anyModifierActive && !numLockState?.locked) {
            return "input-caps-on";
        } else if (anyModifierActive && numLockState?.locked) {
            return "input-combo-on";
        }

        if (capsLockState?.locked && numLockState?.locked) {
            return "input-combo-on";
        } else if (capsLockState?.locked) {
            return "input-caps-on";
        } else if (numLockState?.locked) {
            return "input-num-on";
        } else {
            return "input-caps-on";
        }
    }

    Component {
        id: kiComponent
        KeyboardIndicator.KeyState { }
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
            enabled: anyActive
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

    switchWidth: Kirigami.Units.gridUnit * 12
    switchHeight: Kirigami.Units.gridUnit * 12

    Plasmoid.status: anyActive ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

    toolTipSubText: {
        let text = [];

        if (root.altState.latched) {
            text.push(i18n("Alt is latched"))
        } else if (root.altState.locked) {
            text.push(i18n("Alt is locked"))
        }

        if (root.shiftState.latched) {
            text.push(i18n("Shift is latched"))
        } else if (root.shiftState.locked) {
            text.push(i18n("Shift is locked"))
        }

        if (root.controlState.latched) {
            text.push(i18n("Control is latched"))
        } else if (root.controlState.locked) {
            text.push(i18n("Control is locked"))
        }

        if (root.metaState.latched) {
            text.push(i18n("Meta is latched"))
        } else if (root.metaState.locked) {
            text.push(i18n("Meta is locked"))
        }

        if (root.altGrState.latched) {
            text.push(i18n("Alt Gr is latched"))
        } else if (root.altGrState.locked) {
            text.push(i18n("Alt Gr is locked"))
        }

        if (root.capsLockState?.locked) {
            text.push(i18n("Caps Lock activated"));
        }
        if (root.numLockState?.locked) {
            text.push(i18n("Num Lock activated"));
        }
        if (text.length > 0) {
            // Not using KUIT markup for these newline characters because those
            // get translated into HTML, and this text is displayed in the applet's
            // tooltip which does not render HTML at all for security reasons
            return text.join("\n");
        } else {
            return i18n("No lock keys activated");
        }
    }
}
