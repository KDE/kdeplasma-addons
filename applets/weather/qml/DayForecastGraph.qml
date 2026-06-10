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

    property var futureDays: null
    property var futureDaysPoints: null
    property var metaData: null

    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    Graphs.GraphsView {
        id: forecastGraph

        implicitHeight: root.preferredGraphHeight

        Layout.fillHeight: true
        Layout.fillWidth: true
        marginBottom: Kirigami.Units.largeSpacing
        marginTop: Kirigami.Units.largeSpacing
        marginLeft: forecast.implicitColumnWidth(forecast.leftColumn) / 2 + forecast.neededColumnSpacing + verticalHeader.width
        marginRight: forecast.implicitColumnWidth(forecast.rightColumn) / 2 + forecast.neededColumnSpacing
        clipPlotArea: false
        theme: Graphs.GraphsTheme {
            gridVisible: false
            backgroundVisible: false
            plotAreaBackgroundVisible: false
            seriesColors: [Kirigami.Theme.highlightColor]
        }
        axisX: Graphs.DateTimeAxis {
            min: root.futureDaysPoints.minDate
            max: root.futureDaysPoints.maxDate
            visible: false
            lineVisible: false
        }
        axisY: Graphs.ValueAxis {
            min: root.futureDaysPoints.minTemp
            max: root.futureDaysPoints.maxTemp
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
                        var dayCondition = root.futureDays.data(root.futureDays.index(WeatherData.FutureDays.Day, seriesDelegate.pointIndex), WeatherData.FutureDays.Condition);
                        var nightCondition = root.futureDays.data(root.futureDays.index(WeatherData.FutureDays.Night, seriesDelegate.pointIndex), WeatherData.FutureDays.Condition);
                        if (!!dayCondition && !!nightCondition) {
                            return i18nc("Weather condition summary string", "Day: %1 \nNight: %2", dayCondition, nightCondition);
                        } else if (!!dayCondition || !!nightCondition) {
                            return dayCondition || nightCondition;
                        } else {
                            return "";
                        }
                    }
                }
            }
        }
        Graphs.XYModelMapper {
            orientation: Qt.Horizontal
            model: root.futureDaysPoints
            series: forecastSeries
            count: root.futureDaysPoints.pointsNumber
            xSection: WeatherData.FutureDaysPoints.Timestamp
            ySection: WeatherData.FutureDaysPoints.Temperature
        }
    }

    Item {
        id: forecastView

        implicitWidth: forecast.contentWidth + root.minimalSpacing + verticalHeader.width
        implicitHeight: forecast.contentHeight + root.minimalSpacing + horizontalHeader.height

        Layout.fillWidth: true

        HorizontalHeaderView {
            id: horizontalHeader
            anchors.left: forecast.left
            anchors.top: parent.top
            syncView: forecast
            clip: true
            textRole: "timestamp"
            resizableColumns: false
            interactive: false
            // Check if a night entry exists, as the TableView delegate shows a month/weekday label when no night entry is present.
            model: !!root.futureDays && root.futureDays.daysNumber > 1 && root.futureDays.isNightPresent ? root.futureDays : null

            delegate: PlasmaComponents.Label {
                text: model.timestamp.toLocaleString(Qt.locale(), "ddd") ?? ""
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        VerticalHeaderView {
            id: verticalHeader
            anchors.top: forecast.top
            anchors.left: parent.left
            syncView: forecast
            clip: true
            textRole: "period"
            resizableRows: false
            interactive: false
            model: !!root.futureDays && root.futureDays.isNightPresent ? root.futureDays : null

            delegate: PlasmaComponents.Label {
                text: model.period ?? ""
                textFormat: Text.PlainText
            }
        }

        TableView {
            id: forecast

            interactive: false

            anchors.left: verticalHeader.right
            anchors.top: horizontalHeader.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            model: root.futureDays

            clip: false

            // calculate spacing and implicit width/height when layout changed
            onLayoutChanged: {
                //check if row loaded before calculating row height to prevent rows from being shown incorrectly
                if (isRowLoaded(topRow)) {
                    var rowsHeight = implicitRowHeight(topRow) * rows;
                    neededRowSpacing = Math.max((parent.height - horizontalHeader.height - rowsHeight) / (rows + 1), root.minimalSpacing);
                    implicitHeight = rowsHeight;
                } else {
                    //restore default values if none of rows is loaded (which shows that forecast model is empty)
                    neededRowSpacing = 0;
                    implicitHeight = 0;
                }
                //the same for columns as for rows
                if (isColumnLoaded(leftColumn)) {
                    var columnsWidth = implicitColumnWidth(leftColumn) * columns;
                    neededColumnSpacing = Math.max((parent.width - verticalHeader.width - columnsWidth) / (columns + 1), root.minimalSpacing);
                    implicitWidth = columnsWidth;
                } else {
                    neededColumnSpacing = 0;
                    implicitWidth = 0;
                }
            }

            property real neededRowSpacing: 0
            property real neededColumnSpacing: 0

            anchors.topMargin: neededRowSpacing
            anchors.bottomMargin: neededRowSpacing
            anchors.leftMargin: neededColumnSpacing
            anchors.rightMargin: neededColumnSpacing

            rowSpacing: neededRowSpacing
            columnSpacing: neededColumnSpacing

            delegate: ForecastDelegate {
                showBackground: false
                showConditionIcon: false
                showTimeHeader: !root.futureDays?.isNightPresent
                hasProbability: root.futureDays?.hasProbability
                temperatureUnit: root.metaData?.temperatureUnit || root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
                timeFormat: "ddd"
            }
        }
    }
}
