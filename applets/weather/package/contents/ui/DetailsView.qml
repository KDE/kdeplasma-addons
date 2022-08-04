/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15

import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Loader {
    id: root

    Layout.minimumWidth: grid.implicitWidth
    height: grid.implicitHeight

    property var model

    active: activeFocus
    activeFocusOnTab: isCurrentItem
    asynchronous: true

    sourceComponent: PlasmaExtras.Highlight {
        hovered: true
    }

    Accessible.description: {
        let description = [];
        model.forEach((data) => {
            description.push(`${data.label}: ${data.text};`);
        });
        return description.join(" ");
    }

    GridLayout {
        id: grid

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        rowSpacing: PlasmaCore.Units.smallSpacing

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
