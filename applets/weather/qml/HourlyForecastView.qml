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

    readonly property int preferredIconSize: Kirigami.Units.iconSizes.medium

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    readonly property real preferredCellWidth: root.preferredIconSize + Kirigami.Units.largeSpacing * 2
    readonly property real preferredCellHeight: root.preferredIconSize + 3 * labelFontMetrics.height + Kirigami.Units.largeSpacing * 2

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

    PlasmaComponents.ScrollBar.vertical.policy: PlasmaComponents.ScrollBar.AlwaysOff

    implicitWidth: preferredCellWidth * 7
    implicitHeight: preferredCellHeight + scrollBar.implicitHeight

    focus: true

    contentItem: ListView {
        id: forecasts

        interactive: false

        model: root.futureHours

        clip: true

        anchors.margins: root.minimalSpacing

        spacing: root.minimalSpacing

        orientation: ListView.Horizontal

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
            parent: forecasts.parent
            anchors.top: forecasts.top
            anchors.left: forecasts.right
            anchors.bottom: forecasts.bottom
        }

        delegate: Rectangle {
            color: Qt.alpha(Kirigami.Theme.highlightColor, 0.08)
            implicitWidth: root.preferredCellWidth
            implicitHeight: root.preferredCellHeight
            radius: Kirigami.Units.cornerRadius

            ColumnLayout {
                id: dayDelegate

                visible: !!model.conditionIcon

                anchors.centerIn: parent

                spacing: Math.round(Kirigami.Units.smallSpacing / 2)

                PlasmaComponents.Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        const format = Qt.locale().timeFormat(Locale.ShortFormat);
                        const usesAmPm = format.includes("Ap");
                        if (usesAmPm) {
                            return Qt.formatDateTime(model.timestamp, "h AP");
                        } else {
                            return Qt.formatDateTime(model.timestamp, "HH:mm");
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
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        if (!isNaN(model.highTemp) && !isNaN(model.lowTemp) && !!root.metaData?.temperatureUnit) {
                            let lowTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, model.lowTemp, root.metaData.temperatureUnit, true, true);
                            let highTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, model.highTemp, root.metaData.temperatureUnit, true, true);
                            return i18nc("High and low temperature wrapper", "%1/<font color='%2'>%3</font>", highTemp, Kirigami.Theme.disabledTextColor.toString(), lowTemp);
                        }
                        if (!isNaN(model.highTemp) && !!root.metaData?.temperatureUnit) {
                            return Util.temperatureToDisplayString(root.displayTemperatureUnit, model.highTemp, root.metaData.temperatureUnit, true, true);
                        }

                        if (!isNaN(model.lowTemp) && !!root.metaData?.temperatureUnit) {
                            return Util.temperatureToDisplayString(root.displayTemperatureUnit, model.lowTemp, root.metaData.temperatureUnit, true, true);
                        }
                        return i18nc("Short for no data available", "-");
                    }
                    textFormat: Text.RichText
                    visible: !isNaN(model.highTemp) || !futureDays.isNightPresent
                    Layout.preferredHeight: labelFontMetrics.height
                    font.family: Kirigami.Theme.smallFont.family
                    font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                }

                PlasmaComponents.Label {
                    // Position it closer to the weather condition icon
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    // Fixed value, to prevent the emoji font from setting a larger height
                    Layout.preferredHeight: labelFontMetrics.height

                    horizontalAlignment: Text.AlignHCenter
                    text: !!model.conditionProbability ? i18nc("Probability of precipitation in percentage", "☂%1%", model.conditionProbability) : "·"
                    textFormat: Text.PlainText
                    visible: root.futureHours.hasProbability
                    color: Kirigami.Theme.disabledTextColor
                }
            }
        }
    }
}
