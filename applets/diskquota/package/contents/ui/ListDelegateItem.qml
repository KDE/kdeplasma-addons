/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
        spacing: PlasmaCore.Units.gridUnit

        PlasmaCore.IconItem {
            source: iconName
            Layout.alignment: Qt.AlignTop
            width: PlasmaCore.Units.iconSizes.medium
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
