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

    property var metaData: null
    property var futureDaysPoints: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    property int horizontalLabelsCount: 5
    property int verticalLabelsCount: 4

    readonly property real minimalSpacing: Kirigami.Units.largeSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    property int dateTimeSection: WeatherData.FutureDaysPoints.Timestamp

    property int currentIndex: 0

    component SeriesDefinition: QtObject {
        property string name
        property bool visible
        property color color
        property int ySection
        property bool labelVisible
        property string legendText
        property var labelTextFunc
    }

    property list<SeriesDefinition> seriesDefinitions: [
        SeriesDefinition {
            name: "generalTemp"
            visible: !root.futureDaysPoints?.highLowTempPresent
            color: "red"
            ySection: WeatherData.FutureDaysPoints.GeneralTemp
            legendText: i18n("General Temperature")
            labelTextFunc: function (pointIndex) {
                if (!root.futureDaysPoints) {
                    return "";
                }
                const generalTemp = root.futureDaysPoints.displayTemperature(pointIndex, ySection);
                if (isNaN(generalTemp)) {
                    return "";
                }
                return Util.temperatureToDisplayString(root.displayTemperatureUnit, generalTemp, root.metaData.temperatureUnit);
            }
        },
        SeriesDefinition {
            name: "highTemp"
            visible: root.futureDaysPoints?.highLowTempPresent ?? false
            color: "orange"
            ySection: WeatherData.FutureDaysPoints.HighTemp
            legendText: i18n("High Temperature")
            labelTextFunc: function (pointIndex) {
                if (!root.futureDaysPoints) {
                    return "";
                }
                const highTemp = root.futureDaysPoints.displayTemperature(pointIndex, ySection);
                if (isNaN(highTemp)) {
                    return "";
                }
                return Util.temperatureToDisplayString(root.displayTemperatureUnit, highTemp, root.metaData.temperatureUnit);
            }
        },
        SeriesDefinition {
            name: "lowTemp"
            visible: root.futureDaysPoints?.highLowTempPresent ?? false
            color: "deepskyblue"
            ySection: WeatherData.FutureDaysPoints.LowTemp
            legendText: i18n("Low Temperature")
            labelTextFunc: function (pointIndex) {
                if (!root.futureDaysPoints) {
                    return "";
                }
                const lowTemp = root.futureDaysPoints.displayTemperature(pointIndex, ySection);
                if (isNaN(lowTemp)) {
                    return "";
                }
                return Util.temperatureToDisplayString(root.displayTemperatureUnit, lowTemp, root.metaData.temperatureUnit);
            }
        },
        SeriesDefinition {
            name: "probability"
            visible: root.futureDaysPoints?.hasProbability ?? false
            color: "gray"
            ySection: WeatherData.FutureDaysPoints.ConditionProbability
            legendText: i18n("Condition Probability")
            labelTextFunc: function (pointIndex) {
                if (!root.futureDaysPoints) {
                    return "";
                }
                const conditionProbability = root.futureDaysPoints.displayConditionProbability(pointIndex);
                if (isNaN(conditionProbability)) {
                    return "";
                }
                return Util.percentToDisplayString(conditionProbability);
            }
        }
    ]

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        // Use custom graph labels because GraphView does not support custom label sizing
        // or displaying labels on both sides of the graph.
        ForecastGraphLabels {
            id: tempLabels
            Layout.fillHeight: true
            Layout.topMargin: forecastLine.timestampLabelHeight + forecastGraph.marginTop
            Layout.bottomMargin: forecastGraph.marginBottom
            max: root.futureDaysPoints?.maxTemp ?? 0
            min: root.futureDaysPoints?.minTemp ?? 0
            labelsCount: root.verticalLabelsCount
            formatter: function (temp) {
                return !!root.metaData ? Util.temperatureToDisplayString(root.displayTemperatureUnit, temp, root.metaData.temperatureUnit) : "";
            }
        }

        Item {
            id: graphItem
            Layout.fillWidth: true
            Layout.fillHeight: true

            implicitHeight: forecastGraph.implicitHeight + forecastLine.timestampLabelHeight

            visible: !!root.futureDaysPoints

            ForecastGraphLabels {
                id: dateLabels
                height: forecastLine.timestampLabelHeight
                width: forecastGraph.plotArea.width
                anchors.right: forecastGraph.right
                anchors.left: forecastGraph.left
                anchors.bottom: forecastGraph.top
                anchors.rightMargin: forecastGraph.marginRight
                anchors.leftMargin: forecastGraph.marginLeft
                max: forecastGraph.axisX.visualMax
                min: forecastGraph.axisX.visualMin
                labelsCount: root.horizontalLabelsCount
                horizontal: true
                spacing: root.minimalSpacing
                visible: !forecastGraph.hovered
                formatter: function (timestamp) {
                    const date = new Date(timestamp);
                    const format = Qt.locale().dateFormat(Locale.ShortFormat);
                    return Qt.formatDateTime(date, format);
                }
            }

            // Use a custom graph grid because DateTimeAxis does not allow
            // arbitrary tick positioning, causing labels to not align with the grid.
            ForecastGraphGrid {
                anchors.fill: forecastGraph
                anchors.topMargin: forecastGraph.marginTop
                anchors.bottomMargin: forecastGraph.marginBottom
                anchors.rightMargin: forecastGraph.marginRight
                anchors.leftMargin: forecastGraph.marginLeft

                horizontalLinesNumber: root.verticalLabelsCount
                verticalLinesNumber: root.horizontalLabelsCount
            }

            ForecastGraph {
                id: forecastGraph
                anchors.topMargin: forecastLine.timestampLabelHeight

                anchors.fill: parent

                implicitHeight: root.preferredGraphHeight + marginTop + marginBottom

                marginLeft: root.minimalSpacing
                marginRight: marginLeft

                xModelRow: root.dateTimeSection

                forecastSeries: root.seriesDefinitions

                metaData: root.metaData

                minDate: root.futureDaysPoints?.minDate ?? new Date()
                maxDate: root.futureDaysPoints?.maxDate ?? new Date()

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit

                pointsModel: root.futureDaysPoints
            }

            ForecastGraphLine {
                id: forecastLine

                anchors.fill: parent

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit

                graphMarginLeft: forecastGraph.marginLeft
                graphMarginRight: forecastGraph.marginRight
                graphMarginBottom: forecastGraph.marginBottom
                graphMarginTop: forecastGraph.marginTop

                graphHovered: forecastGraph.hovered

                graphVisualMaxX: forecastGraph.axisX.visualMax
                graphVisualMinX: forecastGraph.axisX.visualMin

                currentPointIndex: forecastGraph.currentPointIndex
                currentPointDateX: forecastGraph.currentPointDateX
                currentPointValues: forecastGraph.currentPointValues

                seriesDefinitions: root.seriesDefinitions

                formatter: function (timestamp) {
                    const date = new Date(timestamp);
                    const format = Qt.locale().dateFormat(Locale.ShortFormat);
                    return Qt.formatDateTime(date, format);
                }

                hasProbability: root.futureDaysPoints?.hasProbability ?? false
                highLowTempPresent: root.futureDaysPoints?.highLowTempPresent ?? false

                maxTemp: root.futureDaysPoints?.maxTemp ?? 0
                minTemp: root.futureDaysPoints?.minTemp ?? 0
            }
        }

        ForecastGraphLabels {
            id: percentLabels
            Layout.fillHeight: true
            Layout.topMargin: forecastLine.timestampLabelHeight + forecastGraph.marginTop
            Layout.bottomMargin: forecastGraph.marginBottom
            leftAlign: true
            max: 100
            min: 0
            labelsCount: root.verticalLabelsCount
            visible: root.futureDaysPoints?.hasProbability ?? false
            formatter: function (conditionProbability) {
                return Util.percentToDisplayString(conditionProbability);
            }
        }
    }

    ForecastGraphLegend {
        Layout.alignment: Qt.AlignHCenter
        Layout.bottomMargin: Kirigami.Units.largeSpacing

        forecastLegendData: {
            const forecastLegend = [];

            for (let i = 0; i < root.seriesDefinitions.length; i++) {
                const series = root.seriesDefinitions[i];

                // Only add the series to the legend if it is currently visible
                if (series.visible) {
                    const legendItem = {
                        label: series.legendText,
                        color: series.color
                    };
                    forecastLegend.push(legendItem);
                }
            }

            return forecastLegend;
        }
    }
}
