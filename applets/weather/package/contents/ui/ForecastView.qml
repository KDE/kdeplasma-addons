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

RowLayout {
    id: root

    property alias model: repeater.model
    readonly property int preferredIconSize: PlasmaCore.Units.iconSizes.large
    readonly property bool hasContent: model && model.length > 0

    Layout.minimumWidth: implicitWidth

    spacing: PlasmaCore.Units.smallSpacing

    Repeater {
        id: repeater

        delegate: ColumnLayout {

            Layout.fillWidth: true
            Layout.preferredWidth: Math.max(preferredIconSize, periodLabel.implicitWidth)

            PlasmaComponents.Label {
                id: periodLabel
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                text: modelData.period.replace(" nt", "")
            }

            PlasmaCore.IconItem {
                Layout.fillWidth: true
                Layout.preferredHeight: preferredIconSize
                Layout.preferredWidth: preferredIconSize

                source: modelData.icon

                PlasmaCore.ToolTipArea {
                    id: iconToolTip
                    anchors.fill: parent
                    mainText: modelData.condition
                }
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: modelData.tempHigh || i18nc("Short for no data available", "-")
            }
            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: modelData.tempLow || i18nc("Short for no data available", "-")
            }
        }
    }
}
