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
import org.kde.plasma.extras as PlasmaExtras

ColumnLayout {
    id: root

    property var futureHours: null
    property var futureDays: null
    property var metaData: null

    property int displayTemperatureUnit: 0

    property string forecastViewTitle: (!!futureDays && futureDays.daysNumber > 0) ? i18ncp("Forecast period timeframe", "1 Day", "%1 Days", futureDays.daysNumber) : ""

    ColumnLayout {
        id: hourlyForecast

        visible: !!root.futureHours && root.futureHours.hoursNumber > 0
        Kirigami.Heading {
            Layout.fillWidth: true
            level: 3
            text: i18n("Hourly Forecast")
        }

        HourlyForecastView {
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true

            futureHours: root.futureHours
            metaData: root.metaData
            displayTemperatureUnit: root.displayTemperatureUnit
        }
    }

    ColumnLayout {
        id: dayForecast

        visible: !!root.futureDays && root.futureDays.daysNumber > 0
        Kirigami.Heading {
            Layout.fillWidth: true
            level: 3
            text: i18n(root.futureDays.daysNumber + " Day Forecast")
        }

        DayForecastView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            futureDays: root.futureDays
            metaData: root.metaData
            displayTemperatureUnit: root.displayTemperatureUnit
        }
    }

    PlasmaExtras.PlaceholderMessage {
        visible: !hourlyForecast.visible && !dayForecast.visible

        Layout.alignment: Qt.AlignCenter
        Layout.margins: Kirigami.Units.largeSpacing
        // Sets a minimum width for the placeholder tab
        Layout.preferredWidth: Kirigami.Units.gridUnit * 15

        text: i18nc("@info:placeholder", "Unable to load weather forecast")
        // TODO: Add a link to the bug-report url, which is now not possible to access within the placeholder
        explanation: i18nc("@info:usagetip", "There may be a technical issue with the weather provider. If the issue persists for longer than a day, submit a bug report.")
    }
}
