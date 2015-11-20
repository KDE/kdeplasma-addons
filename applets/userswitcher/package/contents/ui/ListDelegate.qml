/*
 *  Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

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

    property alias containsMouse: area.containsMouse

    property Item highlight

    Layout.fillWidth: true

    height: row.height + 2 * units.smallSpacing

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
        width: parent.width - 2 * units.smallSpacing
        spacing: units.smallSpacing

        PlasmaCore.IconItem {
            id: icon
            // gosh, there needs to be a Layout.fixedWidth
            Layout.minimumWidth: units.iconSizes.medium
            Layout.maximumWidth: units.iconSizes.medium
            Layout.minimumHeight: units.iconSizes.medium
            Layout.maximumHeight: units.iconSizes.medium

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

            PlasmaComponents.Label {
                id: label
                Layout.fillWidth: true
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
            }

            PlasmaComponents.Label {
                id: sublabel
                Layout.fillWidth: true
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
                opacity: 0.6
                font: theme.smallestFont
                visible: text !== ""
            }
        }
    }
}
