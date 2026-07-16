/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtGraphs

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

Item {
    id: root

    required property int generalTempSection
    required property int highTempSection
    required property int lowTempSection
    required property int conditionProbabilitySection
    required property int dateTimeSection

    required property color highTempSeriesColor
    required property color lowTempSeriesColor
    required property color generalTempSeriesColor
    required property color conditionProbabilitySeriesColor

    required property int invalidUnit
    required property int displayTemperatureUnit

    required property date minDate
    required property date maxDate

    property real currentPointDateX
    property real currentPointGeneralTempY
    property real currentPointHighTempY
    property real currentPointLowTempY
    property real currentPointConditionProbabilityY

    property int totalDays: 0

    property bool hovered: false

    property bool hasProbability: false

    property var metaData: null
    property var pointsModel: null

    property int currentPointIndex: -1

    property alias marginBottom: forecastGraph.marginBottom
    property alias marginTop: forecastGraph.marginTop
    property alias marginRight: forecastGraph.marginRight
    property alias marginLeft: forecastGraph.marginLeft

    property alias axisX: forecastGraph.axisX
    property alias axisY: forecastGraph.axisY

    property alias plotArea: forecastGraph.plotArea

    clip: true

    //Initialize axis min/max values only when pointsModel is
    // to prevent warnings
    onPointsModelChanged: {
        forecastGraph.axisX.min = root.minDate;
        forecastGraph.axisX.max = root.maxDate;
    }

    GraphsView {
        id: forecastGraph
        marginBottom: Kirigami.Units.largeSpacing
        marginTop: Kirigami.Units.largeSpacing
        anchors.fill: parent

        clipPlotArea: false

        theme: GraphsTheme {
            gridVisible: false
            backgroundVisible: false
            plotAreaBackgroundVisible: false
            grid.mainColor: Kirigami.Theme.activeBackgroundColor
        }

        axisX: DateTimeAxis {
            visible: false
            lineVisible: false
            zoom: root.totalDays
            pan: 0
        }

        axisY: ValueAxis {
            visible: false
            lineVisible: false
            min: 0
            max: 100
        }

        SplineSeries {
            id: generalTempSeries
            visible: !root.pointsModel?.highLowTempPresent ?? false
            width: 3
            color: root.generalTempSeriesColor
            pointDelegate: Item {
                id: generalTempDelegate

                property real pointValueX
                property real pointValueY
                property int pointIndex

                Rectangle {
                    id: generalTempPoint
                    anchors.centerIn: parent
                    width: Kirigami.Units.gridUnit * 0.5
                    height: width
                    radius: width * 0.5
                    color: root.generalTempSeriesColor

                    visible: root.currentPointIndex === generalTempDelegate.pointIndex && hoverHandler.hovered

                    onVisibleChanged: {
                        if (visible) {
                            root.currentPointDateX = generalTempDelegate.pointValueX;
                            root.currentPointGeneralTempY = generalTempDelegate.pointValueY;
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

        XYModelMapper {
            orientation: Qt.Horizontal
            ySection: root.generalTempSection
            xSection: root.dateTimeSection
            model: root.pointsModel || null
            series: generalTempSeries
        }

        SplineSeries {
            id: highTempSeries
            visible: root.pointsModel?.highLowTempPresent ?? false
            width: 3
            color: root.highTempSeriesColor
            pointDelegate: Item {
                id: highTempDelegate

                property real pointValueX
                property real pointValueY
                property int pointIndex

                Rectangle {
                    id: highTempPoint
                    anchors.centerIn: parent
                    width: Kirigami.Units.gridUnit * 0.5
                    height: width
                    radius: width * 0.5
                    color: root.highTempSeriesColor

                    visible: root.currentPointIndex === highTempDelegate.pointIndex && hoverHandler.hovered

                    onVisibleChanged: {
                        if (visible) {
                            root.currentPointDateX = highTempDelegate.pointValueX;
                            root.currentPointHighTempY = highTempDelegate.pointValueY;
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

        XYModelMapper {
            orientation: Qt.Horizontal
            ySection: root.highTempSection
            xSection: root.dateTimeSection
            model: root.pointsModel || null
            series: highTempSeries
        }

        SplineSeries {
            id: lowTempSeries
            visible: root.pointsModel?.highLowTempPresent ?? false
            width: 3
            color: root.lowTempSeriesColor
            pointDelegate: Item {
                id: lowTempDelegate

                property real pointValueX
                property real pointValueY
                property int pointIndex

                Rectangle {
                    id: lowTempPoint
                    anchors.centerIn: parent
                    width: Kirigami.Units.gridUnit * 0.5
                    height: width
                    radius: width * 0.5
                    color: root.lowTempSeriesColor

                    visible: root.currentPointIndex === lowTempDelegate.pointIndex && hoverHandler.hovered

                    onVisibleChanged: {
                        if (visible) {
                            root.currentPointDateX = lowTempDelegate.pointValueX;
                            root.currentPointLowTempY = lowTempDelegate.pointValueY;
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

        XYModelMapper {
            orientation: Qt.Horizontal
            xSection: root.dateTimeSection
            ySection: root.lowTempSection
            model: root.pointsModel || null
            series: lowTempSeries
        }

        SplineSeries {
            id: conditionProbabilitySeries
            visible: !!root.pointsModel && root.hasProbability
            width: 3
            color: root.conditionProbabilitySeriesColor
            pointDelegate: Item {
                id: conditionProbabilityDelegate

                property real pointValueX
                property real pointValueY
                property int pointIndex

                Rectangle {
                    id: conditionProbabilityPoint
                    anchors.centerIn: parent
                    width: Kirigami.Units.gridUnit * 0.5
                    height: width
                    radius: width * 0.5
                    color: root.conditionProbabilitySeriesColor

                    visible: root.currentPointIndex === conditionProbabilityDelegate.pointIndex && hoverHandler.hovered

                    onVisibleChanged: {
                        if (visible) {
                            root.currentPointDateX = conditionProbabilityDelegate.pointValueX;
                            root.currentPointConditionProbabilityY = conditionProbabilityDelegate.pointValueY;
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

        XYModelMapper {
            orientation: Qt.Horizontal
            xSection: root.dateTimeSection
            ySection: root.conditionProbabilitySection
            model: root.pointsModel || null
            series: conditionProbabilitySeries
        }

        HoverHandler {
            id: hoverHandler

            onHoveredChanged: {
                if (!hovered) {
                    root.hovered = false;
                }
            }

            onPointChanged: {
                if (!root.pointsModel || !hovered) {
                    root.hovered = false;
                    root.currentPointIndex = -1;
                    return;
                }

                const plotWidth = forecastGraph.plotArea.width;
                const xInPlot = point.position.x - root.marginLeft;

                // Ignore the cursor outside the graph area.
                if (xInPlot < 0 || xInPlot > plotWidth) {
                    root.hovered = false;
                    root.currentPointIndex = -1;
                    return;
                }

                const ratio = xInPlot / plotWidth;

                const visualMin = forecastGraph.axisX.visualMin.getTime();
                const visualMax = forecastGraph.axisX.visualMax.getTime();
                const targetTime = visualMin + ratio * (visualMax - visualMin);

                let left = 0;
                let right = root.pointsModel.columnCount() - 1;

                // Binary search for the first timestamp >= targetTime.
                while (left < right) {
                    const mid = Math.floor((left + right) / 2);

                    const time = new Date(root.pointsModel.data(root.pointsModel.index(root.dateTimeSection, mid), Qt.DisplayRole)).getTime();

                    if (time < targetTime) {
                        left = mid + 1;
                    } else {
                        right = mid;
                    }
                }

                let closest = left;

                // Compare with the previous point to find the nearest one.
                if (left > 0) {
                    const previousTime = new Date(root.pointsModel.data(root.pointsModel.index(root.dateTimeSection, left - 1), Qt.DisplayRole)).getTime();
                    const currentTime = new Date(root.pointsModel.data(root.pointsModel.index(root.dateTimeSection, left), Qt.DisplayRole)).getTime();

                    if (Math.abs(previousTime - targetTime) < Math.abs(currentTime - targetTime) || currentTime > visualMax) {
                        closest = left - 1;
                    }
                }

                root.currentPointIndex = closest;
                root.hovered = true;
            }
        }
    }
}
