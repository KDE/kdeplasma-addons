/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtGraphs as Graphs

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

PlasmaComponents.ScrollView {
    id: root

    property var futureHours: null
    property var futureHoursPoints: null
    property var metaData: null

    property int displayTemperatureUnit: 0

    readonly property int preferredIconSize: Kirigami.Units.iconSizes.medium

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous
    readonly property real preferredCellWidth: root.preferredIconSize + Kirigami.Units.largeSpacing * 2
    readonly property real preferredCellHeight: 3 * labelFontMetrics.height + Kirigami.Units.largeSpacing * 2

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
    implicitHeight: preferredCellHeight + preferredGraphHeight

    focus: true

    contentItem: Flickable {
        contentWidth: (root.preferredCellWidth + root.minimalSpacing) * root.futureHours.hoursNumber + root.minimalSpacing
        contentHeight: root.height - root.ScrollBar.horizontal.height

        ColumnLayout {
            anchors.fill: parent
            Graphs.GraphsView {
                id: forecastGraph
                Layout.fillHeight: true
                Layout.fillWidth: true
                marginBottom: Kirigami.Units.largeSpacing
                marginTop: Kirigami.Units.largeSpacing
                marginLeft: root.preferredCellWidth / 2 + root.minimalSpacing
                marginRight: root.preferredCellWidth / 2 + root.minimalSpacing
                clipPlotArea: false
                theme: Graphs.GraphsTheme {
                    gridVisible: false
                    backgroundVisible: false
                    plotAreaBackgroundVisible: false
                    seriesColors: [Kirigami.Theme.highlightColor]
                }
                axisX: Graphs.DateTimeAxis {
                    min: root.futureHoursPoints.minDate
                    max: root.futureHoursPoints.maxDate
                    visible: false
                    lineVisible: false
                }
                axisY: Graphs.ValueAxis {
                    min: root.futureHoursPoints.minTemp
                    max: root.futureHoursPoints.maxTemp
                    visible: false
                    lineVisible: false
                }
                Graphs.LineSeries {
                    id: forecastSeries
                    width: 3
                    pointDelegate: Rectangle {
                        id: seriesDelegate

                        property real pointValueX
                        property real pointValueY
                        property int pointIndex

                        width: Kirigami.Units.iconSizes.small
                        height: Kirigami.Units.iconSizes.small
                        radius: width * 0.5

                        color: pointHover.hovered ? Kirigami.Theme.linkColor : Kirigami.Theme.highlightColor

                        HoverHandler {
                            id: pointHover
                        }

                        PlasmaCore.ToolTipArea {
                            anchors.fill: parent
                            active: pointHover.hovered
                            mainText: {
                                let index = root.futureHours.index(seriesDelegate.pointIndex, 0);
                                return root.futureHours.data(index, WeatherData.FutureHours.Condition);
                            }
                        }
                    }
                }
                Graphs.XYModelMapper {
                    orientation: Qt.Horizontal
                    model: root.futureHoursPoints
                    series: forecastSeries
                    count: root.futureHoursPoints.pointsNumber
                    xSection: WeatherData.FutureHoursPoints.Timestamp
                    ySection: WeatherData.FutureHoursPoints.Temperature
                }
            }

            ListView {
                id: forecastInfo
                Layout.fillWidth: true
                Layout.preferredHeight: root.preferredCellHeight
                model: root.futureHours
                interactive: false

                clip: false

                Layout.margins: root.minimalSpacing

                spacing: root.minimalSpacing

                orientation: ListView.Horizontal

                delegate: Rectangle {
                    implicitWidth: root.preferredCellWidth
                    implicitHeight: root.preferredCellHeight
                    color: "transparent"
                    ColumnLayout {
                        anchors.centerIn: parent
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
                            visible: !isNaN(model.highTemp) || !isNaN(model.lowTemp)
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
    }
}
