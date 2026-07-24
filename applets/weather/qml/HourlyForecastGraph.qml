/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

ColumnLayout {
    id: root

    property var metaData: null
    property var futureHoursPoints: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.largeSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    property int dateTimeSection: WeatherData.FutureHoursPoints.Timestamp

    property int currentIndex: 0

    property int horizontalLabelsCount: 5
    property int verticalLabelsCount: 4

    property list<ForecastGraph.SeriesDefinition> seriesDefinitions: [
        ForecastGraph.SeriesDefinition {
            name: "generalTemp"
            visible: !root.futureHoursPoints?.highLowTempPresent
            color: "red"
            ySection: WeatherData.FutureHoursPoints.GeneralTemp
            legendText: i18n("General Temperature")
            labelTextFunc: function (pointIndex) {
                if (!root.futureHoursPoints) {
                    return "";
                }
                const generalTemp = root.futureHoursPoints.displayTemperature(pointIndex, ySection);
                if (isNaN(generalTemp)) {
                    return "";
                }
                return Util.temperatureToDisplayString(root.displayTemperatureUnit, generalTemp, root.metaData.temperatureUnit);
            }
        },
        ForecastGraph.SeriesDefinition {
            name: "highTemp"
            visible: root.futureHoursPoints?.highLowTempPresent ?? false
            color: "orange"
            ySection: WeatherData.FutureHoursPoints.HighTemp
            legendText: i18n("High Temperature")
            labelTextFunc: function (pointIndex) {
                if (!root.futureHoursPoints) {
                    return "";
                }
                const highTemp = root.futureHoursPoints.displayTemperature(pointIndex, ySection);
                if (isNaN(highTemp)) {
                    return "";
                }
                return Util.temperatureToDisplayString(root.displayTemperatureUnit, highTemp, root.metaData.temperatureUnit);
            }
        },
        ForecastGraph.SeriesDefinition {
            name: "lowTemp"
            visible: root.futureHoursPoints?.highLowTempPresent ?? false
            color: "deepskyblue"
            ySection: WeatherData.FutureHoursPoints.LowTemp
            legendText: i18n("Low Temperature")
            labelTextFunc: function (pointIndex) {
                if (!root.futureHoursPoints) {
                    return "";
                }
                const lowTemp = root.futureHoursPoints.displayTemperature(pointIndex, ySection);
                if (isNaN(lowTemp)) {
                    return "";
                }
                return Util.temperatureToDisplayString(root.displayTemperatureUnit, lowTemp, root.metaData.temperatureUnit);
            }
        },
        ForecastGraph.SeriesDefinition {
            name: "probability"
            visible: root.futureHoursPoints?.hasProbability ?? false
            color: "gray"
            ySection: WeatherData.FutureHoursPoints.ConditionProbability
            legendText: i18n("Condition Probability")
            labelTextFunc: function (pointIndex) {
                if (!root.futureHoursPoints) {
                    return "";
                }
                const conditionProbability = root.futureHoursPoints.displayConditionProbability(pointIndex);
                if (isNaN(conditionProbability)) {
                    return "";
                }
                return Util.percentToDisplayString(conditionProbability);
            }
        }
    ]

    function scrollToIndex(dayIndex) {
        const totalSpan = forecastGraph.axisX.max - forecastGraph.axisX.min;

        // Calculate the width of the visible window based on zoom level
        // (Zoom is totalDays, making the window exactly 1 day wide)
        const windowWidth = totalSpan / forecastGraph.axisX.zoom;

        // Shift left by dayIndex to bring subsequent days into the view window
        forecastGraph.axisX.pan = forecastGraph.axisX.pan - (currentIndex - dayIndex) * windowWidth;

        currentIndex = dayIndex;
    }

    // Initialize the position when the data model maps its min and max bounds
    onFutureHoursPointsChanged: {
        if (!root.futureHoursPoints) {
            return;
        }
        // Wait for properties to bind, then align to the first index (0)
        Qt.callLater(() => {
            // Shift the viewport so its left edge aligns with axisX.min.
            const totalWidth = forecastGraph.axisX.max - forecastGraph.axisX.min;
            const visibleWidth = forecastGraph.axisX.visualMax - forecastGraph.axisX.visualMin;
            forecastGraph.axisX.pan = -(totalWidth - visibleWidth) / 2;
            const pointSpacing = visibleWidth / root.futureHoursPoints.hoursPerDay;
            // Shift the viewport by half a point spacing so points from the
            // next day are not partially visible at the right edge and the leftmost
            // point is not clipped.
            forecastGraph.axisX.pan -= pointSpacing / 2;
            currentIndex = 0;
        });
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        PlasmaComponents.ToolButton {
            id: backButton
            Layout.fillHeight: true
            icon.name: "go-previous"
            visible: (root.futureHoursPoints?.totalDays ?? 0) > 1
            enabled: root.currentIndex > 0
            onClicked: root.scrollToIndex(root.currentIndex - 1)
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }

        // Use custom graph labels because GraphView does not support custom label sizing
        // or displaying labels on both sides of the graph.
        ForecastGraphLabels {
            id: tempLabels
            Layout.fillHeight: true
            Layout.topMargin: forecastLine.timestampLabelHeight + forecastGraph.marginTop
            Layout.bottomMargin: forecastGraph.marginBottom
            max: root.futureHoursPoints?.maxTemp ?? 0
            min: root.futureHoursPoints?.minTemp ?? 0
            labelsCount: root.verticalLabelsCount
            spacing: root.minimalSpacing
            formatter: function (temp) {
                return !!root.metaData ? Util.temperatureToDisplayString(root.displayTemperatureUnit, temp, root.metaData.temperatureUnit) : "";
            }
        }

        Item {
            id: graphItem
            Layout.fillWidth: true
            Layout.fillHeight: true

            // If the percentage labels are hidden, reserve the same space on the right
            // as the temperature labels occupy on the left to keep the graph centered.
            Layout.rightMargin: percentLabels.visible ? 0 : tempLabels.implicitWidth

            implicitHeight: forecastGraph.implicitHeight + forecastLine.timestampLabelHeight

            visible: !!root.futureHoursPoints

            ForecastGraphLabels {
                id: dateLabels
                height: forecastLine.timestampLabelHeight
                width: forecastGraph.plotArea.width
                max: forecastGraph.axisX.visualMax
                min: forecastGraph.axisX.visualMin
                labelsCount: root.horizontalLabelsCount
                horizontal: true
                spacing: root.minimalSpacing
                visible: !forecastGraph.hovered
                formatter: function (timestamp) {
                    const date = new Date(timestamp);
                    const format = Qt.locale().timeFormat(Locale.ShortFormat);
                    return Qt.formatDateTime(date, format);
                }
            }

            // Use a custom graph grid because DateTimeAxis does not allow
            // arbitrary tick positioning, causing labels to not align with the grid.
            ForecastGraphGrid {
                anchors.fill: forecastGraph
                anchors.topMargin: forecastGraph.marginTop
                anchors.bottomMargin: forecastGraph.marginBottom

                horizontalLinesNumber: root.verticalLabelsCount
                verticalLinesNumber: root.horizontalLabelsCount
            }

            ForecastGraph {
                id: forecastGraph
                anchors.topMargin: forecastLine.timestampLabelHeight

                anchors.fill: parent

                implicitHeight: root.preferredGraphHeight + marginTop + marginBottom

                marginLeft: 0
                marginRight: marginLeft

                xModelRow: root.dateTimeSection

                forecastSeries: root.seriesDefinitions

                metaData: root.metaData

                minDate: root.futureHoursPoints?.minDate ?? new Date()

                // Use the end of the last day rather than futureHoursPoints.maxDate so each page displays a full day.
                // This also preserves the layout of the graph.
                maxDate: {
                    if (!root.futureHoursPoints?.minDate) {
                        return new Date();
                    }
                    const maxDate = new Date(root.futureHoursPoints.minDate);
                    maxDate.setDate(maxDate.getDate() + root.futureHoursPoints.totalDays);
                    return maxDate;
                }

                zoom: root.futureHoursPoints?.totalDays ?? 0

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit

                pointsModel: root.futureHoursPoints
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
                    const format = Qt.locale().timeFormat(Locale.ShortFormat);
                    return Qt.formatDateTime(date, format);
                }

                hasProbability: root.futureHoursPoints?.hasProbability ?? false
                highLowTempPresent: root.futureHoursPoints?.highLowTempPresent ?? false

                maxTemp: root.futureHoursPoints?.maxTemp ?? 0
                minTemp: root.futureHoursPoints?.minTemp ?? 0
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
            visible: root.futureHoursPoints?.hasProbability ?? false
            spacing: root.minimalSpacing
            formatter: function (conditionProbability) {
                return Util.percentToDisplayString(conditionProbability);
            }
        }

        PlasmaComponents.ToolButton {
            id: forwardButton
            Layout.fillHeight: true
            icon.name: "go-next"
            visible: (root.futureHoursPoints?.totalDays ?? 0) > 1
            enabled: root.currentIndex < forecastGraph.axisX.zoom - 1
            onClicked: {
                root.scrollToIndex(root.currentIndex + 1);
            }
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }
    }

    ForecastGraphLegend {
        Layout.alignment: Qt.AlignHCenter
        Layout.leftMargin: backButton.width
        Layout.rightMargin: forwardButton.width
        Layout.bottomMargin: Kirigami.Units.largeSpacing

        seriesDefinitions: root.seriesDefinitions
    }

    PlasmaComponents.PageIndicator {
        id: indicator

        count: root.futureHoursPoints?.totalDays ?? 0

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
