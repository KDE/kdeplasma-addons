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

ColumnLayout {
    id: root

    property var futureHours: null
    property var futureHoursPoints: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.largeSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    readonly property int hoursPerDay: root.futureHoursPoints?.hoursPerDay ?? 0
    readonly property int totalDays: root.futureHoursPoints?.totalDays ?? 0
    readonly property int totalHours: root.futureHoursPoints?.totalHours ?? 0
    readonly property real minTemp: root.futureHoursPoints?.minTemp ?? 0

    readonly property real pointSpacing: forecastFlickable.width / Math.max(1, root.hoursPerDay)

    property color highTempColor: "orange"
    property color lowTempColor: "blue"
    property color generalTempColor: "red"

    property int currentIndex: 0

    function scrollToIndex(dayIndex) {
        currentIndex = dayIndex;
        const firstPointIndex = dayIndex * root.hoursPerDay;
        const xPosition = Math.max(0, firstPointIndex * root.pointSpacing);
        const xLastPage = forecastFlickable.contentWidth - forecastFlickable.width;
        forecastFlickable.contentX = Math.min(xPosition, xLastPage);
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        PlasmaComponents.ToolButton {
            Layout.fillHeight: true
            icon.name: "go-previous"
            enabled: root.currentIndex > 0
            onClicked: root.scrollToIndex(root.currentIndex - 1)
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }

        Rectangle {
            id: graphRectangle
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: "transparent"

            implicitWidth: forecastFlickable.implicitWidth
            implicitHeight: forecastFlickable.implicitHeight + forecastLine.timestampLabelHeight

            Flickable {
                id: forecastFlickable

                anchors.fill: parent
                anchors.topMargin: forecastLine.timestampLabelHeight

                implicitHeight: contentHeight
                contentWidth: forecastGraph.implicitWidth
                contentHeight: forecastGraph.implicitHeight
                clip: true
                interactive: false

                // On layout resize, re-evaluate the current scroll position.
                // This ensures the view stays on the same logical page and prevents
                // contentX from exceeding the last valid scroll position after pageWidth changes.
                onWidthChanged: root.scrollToIndex(root.currentIndex)

                ForecastGraph {
                    id: forecastGraph
                    anchors.fill: parent

                    implicitHeight: root.preferredGraphHeight + marginTop + marginBottom
                    implicitWidth: forecastFlickable.width * ((root.totalHours - 1) / root.hoursPerDay) + marginLeft * 2

                    marginLeft: root.pointSpacing / 2
                    marginRight: marginLeft

                    metaData: root.metaData

                    hovered: forecastLine.hovered

                    invalidUnit: root.invalidUnit
                    displayTemperatureUnit: root.displayTemperatureUnit

                    pointsModel: root.futureHoursPoints

                    highTempSeriesColor: root.highTempColor
                    lowTempSeriesColor: root.lowTempColor
                    generalTempSeriesColor: root.generalTempColor

                    dateTimeSection: WeatherData.FutureHoursPoints.Timestamp
                    generalTempSection: WeatherData.FutureHoursPoints.GeneralTemp
                    highTempSection: WeatherData.FutureHoursPoints.HighTemp
                    lowTempSection: WeatherData.FutureHoursPoints.LowTemp

                    currentPointIndex: root.currentIndex * root.hoursPerDay + forecastLine.pointNumber
                }
            }

            ForecastGraphLine {
                id: forecastLine

                anchors.fill: parent

                graphMarginLeft: forecastGraph.marginLeft
                graphMarginRight: forecastGraph.marginRight
                graphMarginBottom: forecastGraph.marginBottom
                graphMarginTop: forecastGraph.marginTop

                generalTempColor: root.generalTempColor
                highTempColor: root.highTempColor
                lowTempColor: root.lowTempColor

                pointSpacing: root.pointSpacing

                highLowTempPresent: root.futureHoursPoints.highLowTempPresent

                maxTemp: root.futureHoursPoints?.maxTemp ?? 0
                minTemp: root.futureHoursPoints?.minTemp ?? 0

                highTemp: {
                    if (!root.futureHours) {
                        return null;
                    }
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    return root.futureHours.data(index, WeatherData.FutureHours.HighTemp) ?? null;
                }

                lowTemp: {
                    if (!root.futureHours) {
                        return null;
                    }
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    return root.futureHours.data(index, WeatherData.FutureHours.LowTemp) ?? null;
                }

                generalTemp: {
                    if (!root.futureHours) {
                        return null;
                    }
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    return root.futureHours.data(index, WeatherData.FutureHours.GeneralTemp) ?? null;
                }

                timestamp: {
                    if (!root.futureHours) {
                        return "";
                    }
                    const format = Qt.locale().timeFormat(Locale.ShortFormat);
                    const usesAmPm = format.includes("Ap");
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    const timestamp = root.futureHours.data(index, WeatherData.FutureHours.Timestamp);
                    Qt.formatDateTime(timestamp, usesAmPm ? "h AP" : "HH:mm");
                }
            }
        }

        PlasmaComponents.ToolButton {
            Layout.fillHeight: true
            icon.name: "go-next"
            enabled: root.currentIndex < root.totalDays - 1
            onClicked: root.scrollToIndex(root.currentIndex + 1)
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }
    }

    PlasmaComponents.PageIndicator {
        id: indicator

        count: root.totalDays

        currentIndex: root.currentIndex

        Layout.alignment: Qt.AlignHCenter

        visible: count > 1

        delegate: Rectangle {
            width: Math.round(Kirigami.Units.gridUnit * 0.5)
            height: width
            radius: width / 2

            color: index === indicator.currentIndex ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
            opacity: index === indicator.currentIndex ? 1.0 : 0.4

            TapHandler {
                id: tapHandler
                onTapped: {
                    root.scrollToIndex(index);
                }
            }

            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.shortDuration
                }
            }
        }
    }
}
