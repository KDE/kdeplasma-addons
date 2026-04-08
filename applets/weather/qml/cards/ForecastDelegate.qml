/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Rectangle {
    id: root

    property bool showTimeHeader: true

    property var metaData: null

    required property bool hasProbability
    required property string timeFormat
    required property int displayTemperatureUnit

    required property date timestamp
    required property var conditionIcon
    required property var condition
    required property var highTemp
    required property var lowTemp
    required property var generalTemp
    required property var conditionProbability

    // Use the minimum possible spacing to prevent the applet from having a slider.
    readonly property int minimalSpacing: 2

    readonly property int verticalPadding: Kirigami.Units.smallSpacing * 2

    visible: {
        const hasConditionIcon = !!conditionIcon && conditionIcon !== "" && conditionIcon !== "weather-none-available";
        const hasCondition = !!condition && condition !== "";
        const hasHighTemp = !isNaN(highTemp);
        const hasLowTemp = !isNaN(lowTemp);
        const hasConditionProbability = !isNaN(conditionProbability);

        return hasConditionIcon || hasCondition || hasHighTemp || hasLowTemp || hasConditionProbability;
    }

    color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.05)
    radius: Kirigami.Units.cornerRadius

    // calculate manually instead of dayDelegate.imlicitHeight so all delegates have the same height regardless of the available data
    implicitHeight: Kirigami.Units.iconSizes.medium + labelFontMetrics.height * (showTimeHeader ? 2 : 1) + labelFontMetrics.height * (hasProbability ? 1 : 0) + minimalSpacing * (showTimeHeader ? 3 : 2) + verticalPadding

    // equalize the vertical spacing and the horizontal spacing for the default System Tray popup size with slider
    implicitWidth: Kirigami.Units.iconSizes.large + 3

    TextMetrics {
        id: labelFontMetrics
        text: "99%" // We want the sizing for the regular font, not emoji
        font: Kirigami.Theme.defaultFont // Explicitly use the actual Label's font even if it's the default one
    }

    ColumnLayout {
        id: dayDelegate

        anchors.centerIn: parent

        spacing: root.minimalSpacing

        PlasmaComponents.Label {
            visible: root.showTimeHeader
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: Qt.formatDateTime(root.timestamp, root.timeFormat)
            textFormat: Text.PlainText
        }

        Kirigami.Icon {
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            source: root.conditionIcon

            PlasmaComponents.ToolTip {
                visible: iconHoverHandler.hovered && !!root.condition
                delay: Kirigami.Units.toolTipDelay
                text: {
                    if (!root.condition) {
                        return "";
                    }
                    if (!root.conditionProbability) {
                        return root.condition;
                    }
                    return i18nc("certain weather condition (probability percentage)", "%1 (%2%)", root.condition, root.conditionProbability);
                }
            }

            HoverHandler {
                id: iconHoverHandler
            }
        }

        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: {
                if (!isNaN(root.highTemp) && !isNaN(root.lowTemp) && !!root.metaData?.temperatureUnit) {
                    let lowTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lowTemp, root.metaData.temperatureUnit, true, true);
                    let highTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, root.highTemp, root.metaData.temperatureUnit, true, true);
                    return i18nc("High and low temperature wrapper", "%1/<font color='%2'>%3</font>", highTemp, Kirigami.Theme.disabledTextColor.toString(), lowTemp);
                }
                if (!isNaN(root.highTemp) && !!root.metaData?.temperatureUnit) {
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, root.highTemp, root.metaData.temperatureUnit, true, true);
                }

                if (!isNaN(root.lowTemp) && !!root.metaData?.temperatureUnit) {
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lowTemp, root.metaData.temperatureUnit, true, true);
                }

                if (!isNaN(root.generalTemp) && !!root.metaData?.temperatureUnit) {
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, root.generalTemp, root.metaData.temperatureUnit, true, true);
                }

                return i18nc("Short for no data available", "-");
            }
            textFormat: Text.RichText
            font: Kirigami.Theme.smallFont
        }

        PlasmaComponents.Label {
            // Position it closer to the weather condition icon
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            // Fixed value, to prevent the emoji font from setting a larger height
            Layout.preferredHeight: labelFontMetrics.height

            text: !!root.conditionProbability ? i18nc("Probability of precipitation in percentage", "☂%1%", root.conditionProbability) : "·"
            textFormat: Text.PlainText
            visible: root.hasProbability
            opacity: 0.75
        }
    }
}
