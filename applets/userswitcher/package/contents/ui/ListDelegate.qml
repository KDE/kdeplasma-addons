/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.19 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

Item {
    id: item

    signal clicked()

    property alias text: label.text
    property alias subText: sublabel.text
    property bool interactive: true
    property alias iconItem: iconItem.children
    readonly property bool containsMouse: area.containsMouse

    property Item highlight

    // sizing: top-down explicit width, bottom-up implicit height
    implicitHeight: row.implicitHeight + 2 * PlasmaCore.Units.smallSpacing
    implicitWidth: row.implicitWidth
    Layout.fillWidth: true

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
            }
        }
    }

    RowLayout {
        id: row
        anchors.centerIn: parent
        width: parent.width - 2 * PlasmaCore.Units.smallSpacing
        spacing: PlasmaCore.Units.smallSpacing

        Item {
            id: iconItem

            Layout.preferredWidth: PlasmaCore.Units.iconSizes.medium
            Layout.preferredHeight: PlasmaCore.Units.iconSizes.medium
            Layout.minimumWidth: Layout.preferredWidth
            Layout.maximumWidth: Layout.preferredWidth
            Layout.minimumHeight: Layout.preferredHeight
            Layout.maximumHeight: Layout.preferredHeight
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
