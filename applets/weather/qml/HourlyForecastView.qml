/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

PlasmaComponents.ScrollView {
    id: root

    property var futureHours: null
    property var metaData: null

    property int displayTemperatureUnit: 0

    readonly property int preferredIconSize: Kirigami.Units.iconSizes.large
    readonly property int preferredCellsMargin: Kirigami.Units.largeSpacing * 2

    readonly property real preferredCellWidth: root.preferredIconSize
    readonly property real preferredCellHeight: root.preferredIconSize + 4 * labelFontMetrics.height + Kirigami.Units.largeSpacing * 2

    //Item to get the metrics of the regular font in a PlasmaComponent.Label
    PlasmaComponents.Label {
        id: helperLabel
        visible: false

        TextMetrics {
            id: labelFontMetrics
            text: "99%" // We want the sizing for the regular font, not emoji
            font: helperLabel.font // Explicitly use the actual Label's font even if it's the default one
        }
    }

    contentWidth: availableWidth - (contentItem as GridView).leftMargin - (contentItem as GridView).rightMargin
    PlasmaComponents.ScrollBar.horizontal.policy: PlasmaComponents.ScrollBar.AlwaysOff

    implicitWidth: preferredCellWidth * 7
    implicitHeight: preferredCellHeight

    focus: true

    contentItem: GridView {
        id: forecasts

        interactive: false

        model: root.futureHours

        clip: true

        cellWidth: root.preferredCellWidth + (width % root.preferredCellWidth) / (Math.floor(width / root.preferredCellWidth))
        cellHeight: root.preferredCellHeight

        delegate: ColumnLayout {
            id: dayDelegate

            visible: !!model.conditionIcon

            width: root.preferredCellWidth
            height: root.preferredCellHeight - root.preferredCellsMargin

            anchors.bottomMargin: root.preferredCellsMargin

            spacing: Math.round(Kirigami.Units.smallSpacing / 2)

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: {
                    const format = Qt.locale().timeFormat(Locale.ShortFormat);
                    const usesAmPm = format.includes("Ap");
                    if (usesAmPm) {
                        return Qt.formatTime(model.time, "h AP");
                    } else {
                        return Qt.formatTime(model.time, "HH:mm");
                    }
                }
                textFormat: Text.PlainText
                Layout.preferredHeight: labelFontMetrics.height
            }

            Kirigami.Icon {
                Layout.preferredHeight: preferredIconSize
                Layout.preferredWidth: preferredIconSize

                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                source: model.conditionIcon

                PlasmaCore.ToolTipArea {
                    id: iconToolTip
                    anchors.fill: parent
                    mainText: {
                        if (!model.condition) {
                            return "";
                        }
                        if (!model.conditionProbability) {
                            return model.condition;
                        }
                        return i18nc("certain weather condition (probability percentage)", "%1 (%2%)", model.condition, model.conditionProbability);
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
                text: !!model.conditionProbability ? i18nc("Probability of precipitation in percentage", "☂%1%", model.conditionProbability) : "·"
                textFormat: Text.PlainText
                visible: root.futureHours.hasProbability
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isNaN(model.highTemp) && !!root.metaData?.temperatureUnit ? Util.temperatureToDisplayString(root.displayTemperatureUnit, model.highTemp, root.metaData.temperatureUnit, true) : i18nc("Short for no data available", "-")
                textFormat: Text.PlainText
                visible: !isNaN(model.highTemp)
                Layout.preferredHeight: labelFontMetrics.height
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isNaN(model.lowTemp) && !!root.metaData?.temperatureUnit ? Util.temperatureToDisplayString(root.displayTemperatureUnit, model.lowTemp, root.metaData.temperatureUnit, true) : i18nc("Short for no data available", "-")
                textFormat: Text.PlainText
                visible: !isNaN(model.lowTemp)
                Layout.preferredHeight: labelFontMetrics.height
            }
        }
    }
}
