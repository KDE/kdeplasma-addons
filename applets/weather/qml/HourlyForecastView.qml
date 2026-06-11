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

PlasmaComponents.ScrollView {
    id: root

    property var futureHours: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    PlasmaComponents.ScrollBar.vertical.policy: PlasmaComponents.ScrollBar.AlwaysOff

    implicitWidth: forecasts.implicitWidth
    implicitHeight: forecasts.implicitHeight + scrollBar.implicitHeight

    focus: true

    ScrollBar.vertical: ScrollBar {
        id: scrollBar
        parent: root.parent
        anchors.top: root.top
        anchors.left: root.right
        anchors.bottom: root.bottom
    }

    contentItem: HourlyForecastList {
        id: forecasts

        anchors.margins: root.minimalSpacing

        spacing: root.minimalSpacing

        metaData: root.metaData
        forecastModel: root.futureHours

        invalidUnit: root.invalidUnit
        displayTemperatureUnit: root.displayTemperatureUnit
    }
}
