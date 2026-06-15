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

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    PlasmaComponents.ScrollBar.vertical.policy: PlasmaComponents.ScrollBar.AlwaysOff

    ScrollBar.vertical: ScrollBar {
        id: scrollBar
        parent: root.parent
        anchors.top: root.top
        anchors.left: root.right
        anchors.bottom: root.bottom
    }

    implicitWidth: (forecastInfo.contentWidth / forecastInfo.count) * 7
    implicitHeight: forecastInfo.implicitHeight + preferredGraphHeight + scrollBar.implicitHeight

    focus: true

    contentItem: Flickable {
        contentWidth: forecastView.implicitWidth
        contentHeight: root.height - root.ScrollBar.horizontal.height

        ColumnLayout {
            id: forecastView
            anchors.fill: parent
            ForecastGraph {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.margins: root.minimalSpacing

                marginLeft: forecastInfo.contentWidth / (forecastInfo.count * 2)
                marginRight: marginLeft

                pointsModel: root.futureHoursPoints

                xSection: WeatherData.FutureHoursPoints.Timestamp
                ySection: WeatherData.FutureHoursPoints.Temperature

                toolTipTextFunction: function (pointIndex) {
                    if (!root.futureHours) {
                        return "";
                    }
                    let index = root.futureHours.index(pointIndex, 0);
                    return root.futureHours.data(index, WeatherData.FutureHours.Condition);
                }
            }

            HourlyForecastList {
                id: forecastInfo

                showConditionIcon: false
                showBackground: false

                Layout.margins: root.minimalSpacing
                Layout.fillWidth: true

                spacing: root.minimalSpacing

                metaData: root.metaData
                forecastModel: root.futureHours

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
            }
        }
    }
}
