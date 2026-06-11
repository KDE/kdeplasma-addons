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

    clip: true

    orientation: ListView.Horizontal

    implicitWidth: contentWidth
    implicitHeight: contentHeight

    contentWidth: contentItem.childrenRect.width
    contentHeight: contentItem.childrenRect.height

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
