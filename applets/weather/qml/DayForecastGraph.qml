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

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    ForecastGraph {
        id: forecastGraph

        implicitHeight: root.preferredGraphHeight

        Layout.fillHeight: true
        Layout.fillWidth: true

        marginLeft: forecast.columnWidth / 2 + forecast.columnSpacing + forecast.verticalHeaderWidth
        marginRight: forecast.columnWidth / 2 + forecast.columnSpacing

        pointsModel: root.futureDaysPoints

        xSection: WeatherData.FutureDaysPoints.Timestamp
        ySection: WeatherData.FutureDaysPoints.Temperature

        toolTipTextFunction: function (pointIndex) {
            var dayCondition = root.futureDays.data(root.futureDays.index(WeatherData.FutureDays.Day, pointIndex), WeatherData.FutureDays.Condition);
            var nightCondition = root.futureDays.data(root.futureDays.index(WeatherData.FutureDays.Night, pointIndex), WeatherData.FutureDays.Condition);
            if (!!dayCondition && !!nightCondition) {
                return i18nc("Weather condition summary string", "Day: %1 \nNight: %2", dayCondition, nightCondition);
            } else if (!!dayCondition || !!nightCondition) {
                return dayCondition || nightCondition;
            } else {
                return "";
            }
        }
    }

    DayForecastView {
        id: forecast

        Layout.fillWidth: true

        showBackground: false
        showConditionIcon: false

        futureDays: root.futureDays
        metaData: root.metaData

        invalidUnit: root.invalidUnit
        displayTemperatureUnit: root.displayTemperatureUnit
    }
}
