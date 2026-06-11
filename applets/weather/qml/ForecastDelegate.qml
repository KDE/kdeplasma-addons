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

Rectangle {
    id: root

    property bool showConditionIcon: true
    property bool showTimeHeader: true
    property bool showBackground: true

    required property bool hasProbability
    required property string timeFormat
    required property int displayTemperatureUnit
    required property int temperatureUnit

    required property date timestamp
    required property var conditionIcon
    required property var condition
    required property var highTemp
    required property var lowTemp
    required property var conditionProbability

    color: showBackground ? Qt.alpha(Kirigami.Theme.highlightColor, 0.08) : "transparent"
    radius: Kirigami.Units.cornerRadius

    implicitHeight: dayDelegate.implicitHeight + Kirigami.Units.largeSpacing * 2
    implicitWidth: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2

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

    ColumnLayout {
        id: dayDelegate

        anchors.centerIn: parent

        spacing: Math.round(Kirigami.Units.smallSpacing / 2)

        PlasmaComponents.Label {
            visible: root.showTimeHeader
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            text: Qt.formatDateTime(root.timestamp, root.timeFormat)
            textFormat: Text.PlainText
        }

        Kirigami.Icon {
            id: conditionIcon
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            source: root.conditionIcon

            visible: root.showConditionIcon

            PlasmaCore.ToolTipArea {
                id: iconToolTip
                anchors.fill: parent
                mainText: {
                    if (!root.condition) {
                        return "";
                    }
                    if (!root.conditionProbability) {
                        return root.condition;
                    }
                    return i18nc("certain weather condition (probability percentage)", "%1 (%2%)", root.condition, root.conditionProbability);
                }
            }
        }

        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            text: {
                if (!isNaN(root.highTemp) && !isNaN(root.lowTemp) && !!root.temperatureUnit) {
                    let lowTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lowTemp, root.temperatureUnit, true, true);
                    let highTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, root.highTemp, root.temperatureUnit, true, true);
                    return i18nc("High and low temperature wrapper", "%1/<font color='%2'>%3</font>", highTemp, Kirigami.Theme.disabledTextColor.toString(), lowTemp);
                }
                if (!isNaN(root.highTemp) && !!root.temperatureUnit) {
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, root.highTemp, root.temperatureUnit, true, true);
                }

                if (!isNaN(root.lowTemp) && !!root.temperatureUnit) {
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lowTemp, root.temperatureUnit, true, true);
                }
                return i18nc("Short for no data available", "-");
            }
            textFormat: Text.RichText
            visible: !isNaN(root.highTemp) || !isNaN(root.lowTemp)
            font.family: Kirigami.Theme.smallFont.family
            font.pixelSize: Kirigami.Theme.smallFont.pixelSize
        }

        PlasmaComponents.Label {
            // Position it closer to the weather condition icon
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            // Fixed value, to prevent the emoji font from setting a larger height
            Layout.preferredHeight: helperLabel.height

            horizontalAlignment: Text.AlignHCenter
            text: !!root.conditionProbability ? i18nc("Probability of precipitation in percentage", "☂%1%", root.conditionProbability) : "·"
            textFormat: Text.PlainText
            visible: root.hasProbability
            color: Kirigami.Theme.disabledTextColor
        }
    }
}
