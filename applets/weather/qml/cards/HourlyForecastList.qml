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

    property var metaData: null
    property var forecastModel: null

    interactive: false

    model: forecastModel

    orientation: ListView.Horizontal

    currentIndex: 0

    implicitHeight: currentItem?.height ?? 0
    implicitWidth: currentItem?.width ?? 0

    delegate: ForecastDelegate {
        metaData: root.metaData
        hasProbability: root.forecastModel?.hasProbability
        displayTemperatureUnit: root.displayTemperatureUnit
        timeFormat: {
            const format = Qt.locale().timeFormat(Locale.ShortFormat);
            const usesAmPm = format.includes("Ap");
            return usesAmPm ? "h AP" : "HH:mm";
        }
    }
}
