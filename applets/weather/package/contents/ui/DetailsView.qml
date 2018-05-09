/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {
    id: root

    property var model

    GridLayout {
        Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

        rowSpacing: units.smallSpacing

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

                sourceComponent: PlasmaComponents.Label {
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

                sourceComponent: PlasmaComponents.Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    text: rowData.text
                }
            }
        }
    }
}
