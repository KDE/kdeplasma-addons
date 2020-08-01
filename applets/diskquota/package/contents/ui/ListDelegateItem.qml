/*
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents // for ListItem
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.plasma.private.diskquota 1.0

PlasmaComponents.ListItem {
    id: quotaItem
    property string mountPoint
    property string details
    property string iconName
    property string usedString
    property string freeString
    property int usage

    onContainsMouseChanged: {
        if (containsMouse) {
            ListView.view.currentIndex = index
        } else {
            ListView.view.currentIndex = -1
        }
    }

    onClicked: {
        diskQuota.openCleanUpTool(mountPoint);
    }

    RowLayout {
        id: contents
        width: parent.width
        spacing: units.gridUnit

        PlasmaCore.IconItem {
            source: iconName
            Layout.alignment: Qt.AlignTop
            width: units.iconSizes.medium
            height: width
        }

        Column {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            RowLayout {
                width: parent.width
                PlasmaComponents3.Label {
                    Layout.fillWidth: true
                    text: details
                }
                PlasmaComponents3.Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    text: freeString
                    opacity: 0.6
                }
            }
            PlasmaComponents3.ProgressBar {
                width: parent.width
                value: usage
                from: 0
                to: 100
                // HACK to make progressbar clickable
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        quotaItem.clicked()
                    }
                }
            }
            PlasmaComponents3.Label {
                anchors.left: parent.left
                text: usedString
                opacity: 0.6
            }
        }
    }
}
