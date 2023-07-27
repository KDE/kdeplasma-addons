/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9
import QtQuick.Controls as QQC2
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.20 as Kirigami

ColumnLayout {
    id: root

    property var model

    GridLayout {
        Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

        rowSpacing: Kirigami.Units.smallSpacing

        Repeater {
            id: labelRepeater

            model: root.model

            delegate: Loader {
                readonly property int rowIndex: index
                readonly property var rowData: modelData

                Layout.minimumWidth: item.Layout.minimumWidth
                Layout.minimumHeight: item.Layout.minimumHeight
                Layout.alignment: item.Layout.alignment
                Layout.preferredWidth: item.Layout.preferredWidth
                Layout.preferredHeight: item.Layout.preferredHeight
                Layout.row: rowIndex
                Layout.column: 0

                sourceComponent: QQC2.Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                    text: rowData.label
                }
            }
        }

        Repeater {
            id: repeater

            model: root.model

            delegate: Loader {
                readonly property int rowIndex: index
                readonly property var rowData: modelData

                Layout.minimumWidth: item.Layout.minimumWidth
                Layout.minimumHeight: item.Layout.minimumHeight
                Layout.alignment: item.Layout.alignment
                Layout.preferredWidth: item.Layout.preferredWidth
                Layout.preferredHeight: item.Layout.preferredHeight
                Layout.row: rowIndex
                Layout.column: 1

                sourceComponent: QQC2.Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    text: rowData.text
                }
            }
        }
    }
}
