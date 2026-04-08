/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

ListView {
    id: root

    required property int invalidUnit
    required property int displayTemperatureUnit

    property bool showConditionIcon: true
    property bool showBackground: true

    property var metaData: null
    property var forecastModel: null

    interactive: false

    model: forecastModel

    orientation: ListView.Horizontal

    currentIndex: 0

    implicitHeight: currentItem?.height ?? 0
    implicitWidth: currentItem?.width ?? 0

    delegate: ForecastDelegate {
        showBackground: root.showBackground
        showConditionIcon: root.showConditionIcon
        hasProbability: root.forecastModel?.hasProbability
        temperatureUnit: root.metaData?.temperatureUnit || root.invalidUnit
        displayTemperatureUnit: root.displayTemperatureUnit
        timeFormat: {
            const format = Qt.locale().timeFormat(Locale.ShortFormat);
            const usesAmPm = format.includes("Ap");
            return usesAmPm ? "h AP" : "HH:mm";
        }
    }
}
