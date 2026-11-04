/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami

PlasmaComponents3.Control {
    id: root

    required property list<Kirigami.Action> actions

    leftPadding: backgroundItem.margins.left
    rightPadding: backgroundItem.margins.right
    topPadding: backgroundItem.margins.top
    bottomPadding: backgroundItem.margins.bottom

    background: KSvg.FrameSvgItem {
        id: backgroundItem
        anchors.fill: parent
        imagePath: "widgets/background"
    }

    contentItem: Row {
        id: rowButton
        spacing: Kirigami.Units.mediumSpacing

        Repeater {
            id: repeater
            model: root.actions
            delegate: PlasmaComponents3.ToolButton {
                required property Kirigami.Action modelData
                action: modelData
                icon.name: action.icon.name
                enabled: action.enabled
            }
        }
    }
}
