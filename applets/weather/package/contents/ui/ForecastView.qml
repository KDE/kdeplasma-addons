/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15

import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

GridLayout {
    id: root

    property alias model: repeater.model
    property bool showNightRow: false
    readonly property int preferredIconSize: PlasmaCore.Units.iconSizes.large
    readonly property bool hasContent: model && model.length > 0

    Layout.minimumWidth: implicitWidth

    columnSpacing: PlasmaCore.Units.smallSpacing
    rowSpacing: PlasmaCore.Units.smallSpacing

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
            Layout.preferredWidth: Math.max(preferredIconSize + PlasmaCore.Units.smallSpacing, periodLabel.implicitWidth)

            PlasmaComponents.Label {
                id: periodLabel
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.fillHeight: isPlaceHolder
                // Hide period titles on the second row (but keep the space for separation and visual alignment)
                opacity: (isPlaceHolder || isFirstRow) ? 1 : 0

                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                text: isPlaceHolder ? modelData.placeholder || "" : modelData.period.replace(" nt", "")
            }

            PlasmaCore.IconItem {
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
