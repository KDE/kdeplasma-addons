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

    required property int xSection
    required property int ySection

    property var pointsModel: null

    property var toolTipTextFunction: null

    marginBottom: Kirigami.Units.largeSpacing
    marginTop: Kirigami.Units.largeSpacing
    clipPlotArea: false

    theme: GraphsTheme {
        gridVisible: false
        backgroundVisible: false
        plotAreaBackgroundVisible: false
        seriesColors: [Kirigami.Theme.highlightColor]
    }

    axisX: DateTimeAxis {
        min: root.pointsModel.minDate
        max: root.pointsModel.maxDate
        visible: false
        lineVisible: false
    }

    axisY: ValueAxis {
        min: root.pointsModel.minTemp
        max: root.pointsModel.maxTemp
        visible: false
        lineVisible: false
    }

    LineSeries {
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
                mainText: root.toolTipTextFunction(seriesDelegate.pointIndex)
            }
        }
    }

    XYModelMapper {
        orientation: Qt.Horizontal
        model: root.pointsModel
        series: forecastSeries
        count: root.pointsModel.pointsNumber
        xSection: root.xSection
        ySection: root.ySection
    }
}
