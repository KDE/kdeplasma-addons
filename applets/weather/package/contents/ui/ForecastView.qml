/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

GridLayout {
    id: root

    property alias model: repeater.model
    property bool showNightRow: false
    readonly property int preferredIconSize: Kirigami.Units.iconSizes.large
    readonly property bool hasContent: model && model.length > 0

    columnSpacing: Kirigami.Units.smallSpacing
    rowSpacing: Kirigami.Units.smallSpacing

    rows: showNightRow ? 2 : 1
    flow: showNightRow ? GridLayout.TopToBottom : GridLayout.LeftToRight

    Repeater {
        id: repeater

        delegate: ColumnLayout {
            id: dayDelegate
            // Allow to set placeholder items by leaving the data empty or setting a label text
            readonly property bool isPlaceHolder: !modelData || modelData.lenght === 0 || !!modelData.placeholder
            readonly property bool isFirstRow: (model.index % root.rows) === 0

            Layout.fillWidth: true

            PlasmaComponents.Label {
                id: periodLabel
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.fillHeight: isPlaceHolder
                // Hide period titles on the second row
                visible: (isPlaceHolder || isFirstRow)

                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                text: isPlaceHolder ? modelData.placeholder || "" : modelData.period.replace(" nt", "")
            }

            Kirigami.Icon {
                Layout.fillWidth: true
                Layout.preferredHeight: preferredIconSize
                Layout.preferredWidth: preferredIconSize
                visible: !isPlaceHolder

                source: isPlaceHolder ? "" : modelData.icon

                PlasmaCore.ToolTipArea {
                    id: iconToolTip
                    anchors.fill: parent
                    mainText: isPlaceHolder ? "" : modelData.condition
                }
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isPlaceHolder && modelData.tempHigh || i18nc("Short for no data available", "-")
                visible: !isPlaceHolder && (modelData.tempHigh || !showNightRow)
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isPlaceHolder && modelData.tempLow || i18nc("Short for no data available", "-")
                visible: !isPlaceHolder && (modelData.tempLow || !showNightRow)
            }
        }
    }
}
