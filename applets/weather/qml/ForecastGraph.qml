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

GraphsView {
    id: root

    required property int generalTempSection
    required property int highTempSection
    required property int lowTempSection
    required property int dateTimeSection

    required property color highTempSeriesColor
    required property color lowTempSeriesColor
    required property color generalTempSeriesColor

    required property int invalidUnit
    required property int displayTemperatureUnit

    property bool hovered: false

    property var metaData: null
    property var pointsModel: null

    property int currentPointIndex: 0

    marginBottom: Kirigami.Units.largeSpacing
    marginTop: Kirigami.Units.largeSpacing
    clipPlotArea: false

    //Initialize axis min/max values only when pointsModel is
    // to prevent warnings
    onPointsModelChanged: {
        if (!!root.pointsModel) {
            axisX.min = root.pointsModel.minDate;
            axisX.max = root.pointsModel.maxDate;
            axisY.min = root.pointsModel.minTemp;
            axisY.max = root.pointsModel.maxTemp;
        }
    }

    theme: GraphsTheme {
        gridVisible: false
        backgroundVisible: false
        plotAreaBackgroundVisible: false
        seriesColors: {
            let colors = [];
            if (root.pointsModel?.highLowTempPresent) {
                colors.push(root.lowTempSeriesColor);
                colors.push(root.highTempSeriesColor);
            } else {
                colors.push(root.generalTempSeriesColor);
            }
            return colors;
        }
    }

    axisX: DateTimeAxis {
        visible: false
        lineVisible: false
    }

    axisY: ValueAxis {
        visible: false
        lineVisible: false
    }

    LineSeries {
        id: generalTempSeries
        width: 3
        visible: !root.pointsModel.highLowTempPresent
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

                visible: root.currentPointIndex === generalTempDelegate.pointIndex && root.hovered

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

    LineSeries {
        id: highTempSeries
        visible: root.pointsModel.highLowTempPresent
        width: 3
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

                visible: root.currentPointIndex === highTempDelegate.pointIndex && root.hovered

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

    LineSeries {
        id: lowTempSeries
        visible: root.pointsModel.highLowTempPresent
        width: 3
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

                visible: root.currentPointIndex === lowTempDelegate.pointIndex && root.hovered

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
}
