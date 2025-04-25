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

    readonly property bool startsAtNight: !model[0] // When the first item is undefined
    readonly property int preferredIconSize: Kirigami.Units.iconSizes.large
    readonly property bool hasContent: model && model.length > 0
    readonly property var rowHasProbability: [...Array(rows).keys()].map(
        row => model.filter((_, index) => index % root.rows == row)
                    .some(item => item?.probability ?? false))

    columnSpacing: 0
    rowSpacing: Kirigami.Units.largeSpacing

    rows: showNightRow ? 2 : 1
    flow: showNightRow ? GridLayout.TopToBottom : GridLayout.LeftToRight

    // Add Day/Night labels as the row headings when there is a night row
    component DayNightLabel: PlasmaComponents.Label {
        visible: root.showNightRow
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        Layout.fillWidth: true
        Layout.preferredWidth: startsAtNight ? Kirigami.Units.largeSpacing : implicitWidth
        font.bold: true
    }

    DayNightLabel {
        text: i18nc("Time of the day (from the duple Day/Night)", "Day")
    }

    DayNightLabel {
        text: i18nc("Time of the day (from the duple Day/Night)", "Night")
        // Save space by moving this label over the night row when possible
        Layout.topMargin: startsAtNight ? -Kirigami.Units.gridUnit : 0
    }

    // Item to get the metrics of the regular font in a PlasmaComponent.Label
    PlasmaComponents.Label {
        id: helperLabel
        visible: false

        TextMetrics {
            id: labelFontMetrics
            text: "99%" // We want the sizing for the regular font, not emoji
            font: helperLabel.font // Explicitly use the actual Label's font even if it's the default one
        }
    }

    Repeater {
        id: repeater

        delegate: ColumnLayout {
            id: dayDelegate

            Layout.fillWidth: true
            spacing: Math.round(Kirigami.Units.smallSpacing / 2)

            PlasmaComponents.Label {
                id: periodLabel
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                // Hide period titles on the second row
                visible: (model.index % root.rows) === 0

                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                text: modelData?.period?.replace(" nt", "") || ""
                textFormat: Text.PlainText
            }

            Kirigami.Icon {
                Layout.fillWidth: true
                Layout.preferredHeight: preferredIconSize
                Layout.preferredWidth: preferredIconSize

                source: modelData?.icon ?? ""

                PlasmaCore.ToolTipArea {
                    id: iconToolTip
                    anchors.fill: parent
                    mainText: {
                        if (!modelData?.condition) {
                            return "";
                        }
                        if (!modelData?.probability) {
                            return modelData.condition;
                        }
                        return i18nc("certain weather condition (probability percentage)",
                                     "%1 (%2%)", modelData.condition, modelData.probability);
                    }
                }
            }

            PlasmaComponents.Label {
                // Position it closer to the weather condition icon
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                Layout.topMargin: -Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing * 2
                // Fixed value, to prevent the emoji font from setting a larger height
                Layout.preferredHeight: labelFontMetrics.height

                horizontalAlignment: Text.AlignHCenter
                text: modelData?.probability ? i18nc("Probability of precipitation in percentage", "☂%1%", modelData.probability) : "·"
                textFormat: Text.PlainText
                visible: modelData && root.rowHasProbability[index % root.rows]
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: modelData ? modelData.tempHigh || i18nc("Short for no data available", "-") : ""
                textFormat: Text.PlainText
                visible: modelData?.tempHigh || !showNightRow
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: modelData ? modelData.tempLow || i18nc("Short for no data available", "-") : ""
                textFormat: Text.PlainText
                visible: modelData?.tempLow || !showNightRow
            }
        }
    }
}
