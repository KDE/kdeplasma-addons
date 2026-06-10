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

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    PlasmaComponents.ScrollBar.vertical.policy: PlasmaComponents.ScrollBar.AlwaysOff

    implicitWidth: (forecastInfo.contentWidth / forecastInfo.count) * 7
    implicitHeight: forecastInfo.implicitHeight + preferredGraphHeight

    focus: true

    contentItem: Flickable {
        contentWidth: forecastView.implicitWidth
        contentHeight: root.height - root.ScrollBar.horizontal.height

        ColumnLayout {
            id: forecastView
            anchors.fill: parent
            Graphs.GraphsView {
                id: forecastGraph
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.margins: root.minimalSpacing
                marginBottom: Kirigami.Units.largeSpacing
                marginTop: Kirigami.Units.largeSpacing
                marginLeft: forecastInfo.implicitWidth / (forecastInfo.count * 2)
                marginRight: marginLeft
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
                Layout.margins: root.minimalSpacing
                model: root.futureHours
                interactive: false

                clip: false

                implicitWidth: contentWidth
                implicitHeight: contentHeight

                contentWidth: contentItem.childrenRect.width
                contentHeight: contentItem.childrenRect.height

                orientation: ListView.Horizontal

                spacing: root.minimalSpacing

                delegate: ForecastDelegate {
                    showBackground: false
                    showConditionIcon: false
                    hasProbability: root.futureHours?.hasProbability
                    temperatureUnit: root.metaData?.temperatureUnit || root.invalidUnit
                    displayTemperatureUnit: root.displayTemperatureUnit
                    timeFormat: {
                        const format = Qt.locale().timeFormat(Locale.ShortFormat);
                        const usesAmPm = format.includes("Ap");
                        return usesAmPm ? "h AP" : "HH:mm";
                    }
                }
            }
        }
    }
}
