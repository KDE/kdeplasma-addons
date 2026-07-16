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

    property color highTempColor: "orange"
    property color lowTempColor: "blue"
    property color generalTempColor: "red"
    property color conditionProbabilityColor: "gray"

    property int currentIndex: 0

    property int horizontalLabelsCount: 5
    property int verticalLabelsCount: 4

    function scrollToIndex(dayIndex) {
        // 1. Calculate the total span of data
        let totalSpan = forecastGraph.axisX.max - forecastGraph.axisX.min;

        // 2. Calculate the width of the visible window based on zoom level
        // (Zoom is totalDays, making the window exactly 1 day wide)
        let windowWidth = totalSpan / forecastGraph.axisX.zoom;

        // 4. Shift left by dayIndex to bring subsequent days into the view window
        forecastGraph.axisX.pan = forecastGraph.axisX.pan - (currentIndex - dayIndex) * windowWidth;

        currentIndex = dayIndex;
    }

    // Initialize the position when the data model maps its min and max bounds
    onFutureHoursChanged: {
        if (!!root.futureHours) {
            // Wait for properties to bind, then align to the first index (0)
            Qt.callLater(() => {
                // DateTimeAxis.pan is a translation from the centered position.
                // Shift the viewport so its left edge aligns with axisX.min.
                const totalWidth = forecastGraph.axisX.max - forecastGraph.axisX.min;
                const visibleWidth = forecastGraph.axisX.visualMax - forecastGraph.axisX.visualMin;
                forecastGraph.axisX.pan = -(totalWidth - visibleWidth) / 2;
                const pointSpacing = visibleWidth / root.futureHoursPoints.hoursPerDay;
                forecastGraph.axisX.pan -= pointSpacing / 2;
                currentIndex = 0;
            });
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        PlasmaComponents.ToolButton {
            id: backButton
            Layout.fillHeight: true
            icon.name: "go-previous"
            enabled: root.currentIndex > 0
            onClicked: root.scrollToIndex(root.currentIndex - 1)
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }

        ForecastGraphLabels {
            id: tempLabels
            Layout.fillHeight: true
            Layout.topMargin: forecastLine.timestampLabelHeight + forecastGraph.marginTop
            Layout.bottomMargin: forecastGraph.marginBottom
            max: root.futureHoursPoints?.maxTemp
            min: root.futureHoursPoints?.minTemp
            labelsCount: root.verticalLabelsCount
            spacing: root.minimalSpacing
            formatter: function (temp) {
                return Util.temperatureToDisplayString(root.displayTemperatureUnit, temp, root.metaData.temperatureUnit);
            }
        }

        Item {
            id: graphItem
            Layout.fillWidth: true
            Layout.fillHeight: true

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

                metaData: root.metaData

                hasProbability: root.futureHours?.hasProbability ?? false

                minDate: root.futureHoursPoints?.minDate ?? new Date()

                // Use the end of the last day rather than futureHoursPoints.maxDate so each page displays a full day.
                // This also preserves the layout of the graph.
                maxDate: {
                    if (!root.futureHoursPoints?.minDate) {
                        return new Date();
                    }
                    var maxDate = new Date(root.futureHoursPoints.minDate);
                    maxDate.setDate(maxDate.getDate() + root.totalDays);
                    return maxDate;
                }

                totalDays: root.totalDays

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit

                pointsModel: root.futureHoursPoints

                highTempSeriesColor: root.highTempColor
                lowTempSeriesColor: root.lowTempColor
                generalTempSeriesColor: root.generalTempColor
                conditionProbabilitySeriesColor: root.conditionProbabilityColor

                dateTimeSection: WeatherData.FutureHoursPoints.Timestamp
                generalTempSection: WeatherData.FutureHoursPoints.GeneralTemp
                highTempSection: WeatherData.FutureHoursPoints.HighTemp
                lowTempSection: WeatherData.FutureHoursPoints.LowTemp
                conditionProbabilitySection: WeatherData.FutureHoursPoints.ConditionProbability
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

                generalTempColor: root.generalTempColor
                highTempColor: root.highTempColor
                lowTempColor: root.lowTempColor
                conditionProbabilityColor: root.conditionProbabilityColor

                graphHovered: forecastGraph.hovered

                graphVisualMaxX: forecastGraph.axisX.visualMax
                graphVisualMinX: forecastGraph.axisX.visualMin

                currentPointDateX: forecastGraph.currentPointDateX
                currentPointGeneralTempY: forecastGraph.currentPointGeneralTempY
                currentPointHighTempY: forecastGraph.currentPointHighTempY
                currentPointLowTempY: forecastGraph.currentPointLowTempY
                currentPointConditionProbabilityY: forecastGraph.currentPointConditionProbabilityY

                hasProbability: root.futureHours?.hasProbability ?? false
                highLowTempPresent: root.futureHoursPoints?.highLowTempPresent ?? false

                maxTemp: root.futureHoursPoints?.maxTemp ?? 0
                minTemp: root.futureHoursPoints?.minTemp ?? 0

                generalTempText: {
                    if (!root.futureHours) {
                        return "";
                    }
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    let generalTemp = root.futureHours.data(index, WeatherData.FutureHours.GeneralTemp);
                    if (!generalTemp) {
                        return "";
                    }
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, generalTemp, root.metaData.temperatureUnit);
                }

                highTempText: {
                    if (!root.futureHours) {
                        return "";
                    }
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    let highTemp = root.futureHours.data(index, WeatherData.FutureHours.HighTemp);
                    if (!highTemp) {
                        return "";
                    }
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, highTemp, root.metaData.temperatureUnit);
                }

                lowTempText: {
                    if (!root.futureHours) {
                        return "";
                    }
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    let lowTemp = root.futureHours.data(index, WeatherData.FutureHours.LowTemp);
                    if (!lowTemp) {
                        return "";
                    }
                    return Util.temperatureToDisplayString(root.displayTemperatureUnit, lowTemp, root.metaData.temperatureUnit);
                }

                conditionProbabilityText: {
                    if (!root.futureHours) {
                        return "";
                    }
                    let index = root.futureHours.index(forecastGraph.currentPointIndex, 0);
                    let conditionProbability = root.futureHours.data(index, WeatherData.FutureHours.ConditionProbability);
                    if (!conditionProbability) {
                        return "";
                    }
                    return Util.percentToDisplayString(conditionProbability);
                }
            }
        }

        ForecastGraphLabels {
            id: percentLabes
            Layout.fillHeight: true
            Layout.topMargin: forecastLine.timestampLabelHeight + forecastGraph.marginTop
            Layout.bottomMargin: forecastGraph.marginBottom
            leftAllign: true
            max: 100
            min: 0
            labelsCount: root.verticalLabelsCount
            visible: forecastGraph.hasProbability
            spacing: root.minimalSpacing
            formatter: function (conditionProbability) {
                return Util.percentToDisplayString(conditionProbability);
            }
        }

        PlasmaComponents.ToolButton {
            id: forwardButton
            Layout.fillHeight: true
            icon.name: "go-next"
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

        forecastLegendData: {
            let forecastLegend = [];
            if (!forecastGraph.highLowTempPresent) {
                let generalTempData = {
                    label: i18n("General Temperature"),
                    color: root.generalTempColor
                };
                forecastLegend.push(generalTempData);
            } else {
                let highTempData = {
                    label: i18n("High Temperature"),
                    color: root.highTempColor
                };
                let lowTempData = {
                    label: i18n("Low Temperature"),
                    color: root.lowTempColor
                };
                forecastLegend.push(highTempData);
                forecastLegend.push(lowTempData);
            }
            if (forecastGraph.hasProbability) {
                let conditionProbability = {
                    label: i18n("Condition Probability"),
                    color: root.conditionProbabilityColor
                };
                forecastLegend.push(conditionProbability);
            }
            return forecastLegend;
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
