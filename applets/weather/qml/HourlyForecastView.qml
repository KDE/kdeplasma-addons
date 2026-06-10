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

    contentItem: ListView {
        id: forecasts

        interactive: false

        model: root.futureHours

        clip: true

        anchors.margins: root.minimalSpacing

        spacing: root.minimalSpacing

        orientation: ListView.Horizontal

        implicitWidth: contentWidth + root.minimalSpacing
        implicitHeight: contentHeight + root.minimalSpacing

        contentWidth: contentItem.childrenRect.width
        contentHeight: contentItem.childrenRect.height

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
            parent: forecasts.parent
            anchors.top: forecasts.top
            anchors.left: forecasts.right
            anchors.bottom: forecasts.bottom
        }

        delegate: ForecastDelegate {
            hasProbability: root.futureHours?.hasProbability
            temperatureUnit: root.metaData?.temperatureUnit || root.invalidUnit
            displayTemperatureUnit: root.displayTemperatureUnit
            timeFormat: {
                const format = Qt.locale().timeFormat(Locale.ShortFormat);
                const usesAmPm = format.includes("Ap");
                return usesAmPm ? "h AP" : "HH:mm";
            }
        }
    }
}
