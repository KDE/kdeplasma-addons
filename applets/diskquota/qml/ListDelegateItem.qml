/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3

PlasmaComponents3.ItemDelegate {
    id: quotaItem
    required property string mountPoint
    required property string details
    required property string used
    required property string free
    required property int usage
    required property int index

    onHoveredChanged: {
        if (hovered) {
            ListView.view.currentIndex = index
        } else if (ListView.isCurrentItem) {
            ListView.view.currentIndex = -1
        }
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.gridUnit

        Kirigami.Icon {
            source: quotaItem.icon.name
            Layout.alignment: Qt.AlignTop
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: implicitWidth
        }

        Column {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            RowLayout {
                width: parent.width
                PlasmaComponents3.Label {
                    Layout.fillWidth: true
                    text: quotaItem.details
                    textFormat: Text.PlainText
                }
                PlasmaComponents3.Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    text: quotaItem.free
                    textFormat: Text.PlainText
                    opacity: 0.75
                }
            }
            PlasmaComponents3.ProgressBar {
                width: parent.width
                value: quotaItem.usage
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
                text: quotaItem.used
                textFormat: Text.PlainText
                opacity: 0.75
            }
        }
    }
}
