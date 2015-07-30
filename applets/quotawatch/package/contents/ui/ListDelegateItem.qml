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
import org.kde.plasma.components 2.0 as Components

import org.kde.plasma.private.quotawatch 0.1

Components.ListItem {
    property string mountPoint
    property string usedString
    property string freeString
    property int usage

    enabled: true
    onContainsMouseChanged: {
        if (containsMouse) {
            ListView.view.currentIndex = index;
        }
    }
    
    onClicked: {
        kRun.openUrl(apps.data[apps.connectedSources[0]].entryPath)
    }
    
    RowLayout {
        id: contents
        anchors.left: parent.left
        anchors.right: parent.right
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
                Components.Label {
                    Layout.fillWidth: true
                    height: paintedHeight
                    text: mountPoint
                }
                Components.Label {
                    Layout.fillWidth: true
                    height: paintedHeight
                    horizontalAlignment: Text.AlignRight
                    text: freeString
                    opacity: 0.6
                }
            }
            Components.ProgressBar {
                width: parent.width
                value: usage
                minimumValue: 0
                maximumValue: 100
            }
            Components.Label {
                height: paintedHeight
                anchors.left: parent.left
                text: usedString
                horizontalAlignment: Text.AlignLeft
                opacity: 0.6
            }
        }
    }
}
