/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

Item {
    id: item

    signal clicked
    signal iconClicked

    property alias text: label.text
    property alias subText: sublabel.text
    property alias icon: icon.source
    // "enabled" also affects all children
    property bool interactive: true
    property bool interactiveIcon: false

    property alias usesPlasmaTheme: icon.usesPlasmaTheme

    property alias containsMouse: area.containsMouse

    property Item highlight

    Layout.fillWidth: true

    height: row.height + 2 * PlasmaCore.Units.smallSpacing

    MouseArea {
        id: area
        anchors.fill: parent
        enabled: item.interactive
        hoverEnabled: true
        onClicked: item.clicked()
        onContainsMouseChanged: {
            if (!highlight) {
                return
            }

            if (containsMouse) {
                highlight.parent = item
                highlight.width = item.width
                highlight.height = item.height
            }

            highlight.visible = containsMouse
        }
    }

    RowLayout {
        id: row
        anchors.centerIn: parent
        width: parent.width - 2 * PlasmaCore.Units.smallSpacing
        spacing: PlasmaCore.Units.smallSpacing

        PlasmaCore.IconItem {
            id: icon
            // gosh, there needs to be a Layout.fixedWidth
            Layout.minimumWidth: PlasmaCore.Units.iconSizes.medium
            Layout.maximumWidth: PlasmaCore.Units.iconSizes.medium
            Layout.minimumHeight: PlasmaCore.Units.iconSizes.medium
            Layout.maximumHeight: PlasmaCore.Units.iconSizes.medium

            MouseArea {
                anchors.fill: parent
                visible: item.interactiveIcon
                cursorShape: Qt.PointingHandCursor
                onClicked: item.iconClicked()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            PlasmaComponents3.Label {
                id: label
                Layout.fillWidth: true
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
            }

            PlasmaComponents3.Label {
                id: sublabel
                Layout.fillWidth: true
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
                opacity: 0.6
                font: PlasmaCore.Theme.smallestFont
                visible: text !== ""
            }
        }
    }
}
