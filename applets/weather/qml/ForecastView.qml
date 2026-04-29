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

ColumnLayout {
    id: root

    property var futureHours: null
    property var futureDays: null
    property var metaData: null

    property int displayTemperatureUnit: 0

    ColumnLayout {
        visible: !!root.futureHours && root.futureHours.hoursNumber > 0
        Kirigami.Heading {
            visible: !!root.futureDays && root.futureDays.daysNumber > 0
            Layout.fillWidth: true
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
        visible: !!root.futureDays && root.futureDays.daysNumber > 0
        Kirigami.Heading {
            visible: !!root.futureHours && root.futureHours.hoursNumber > 0
            Layout.fillWidth: true
            text: i18n("Day Forecast")
        }

        DayForecastView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            futureDays: root.futureDays
            metaData: root.metaData
            displayTemperatureUnit: root.displayTemperatureUnit
        }
    }
}
