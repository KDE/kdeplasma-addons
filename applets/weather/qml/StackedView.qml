/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root

    property var locationHeaderComponent: null
    property var currentConditionsComponent: null

    property bool showHourlyForecast: false
    property string hourlySubHeaderText: ""
    property var hourlyComponent: null

    property var dayComponent: null
    property bool showDayForecast: false

    property var sourceLabelComponent: null

    readonly property alias hourlyItem: stackedHourlyLoader.item
    readonly property alias dayItem: stackedDayLoader.item

    Loader {
        id: locationHeaderLoader
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true
        visible: status == Loader.Ready
        sourceComponent: root.locationHeaderComponent
    }

    Loader {
        id: currentConditionsLoader
        Layout.fillWidth: true
        visible: status == Loader.Ready
        sourceComponent: root.currentConditionsComponent
    }

    ColumnLayout {
        id: hourlyForecast
        Layout.fillWidth: true
        visible: root.showHourlyForecast

        RowLayout {
            spacing: Kirigami.Units.mediumSpacing

            Kirigami.Heading {
                level: 3
                text: i18nc("@title", "Hourly Forecast")
            }

            Kirigami.Heading {
                level: 3
                text: root.hourlySubHeaderText
                opacity: 0.75
            }
        }

        Loader {
            id: stackedHourlyLoader
            Layout.fillWidth: true
            visible: status == Loader.Ready
            active: root.showHourlyForecast
            sourceComponent: root.hourlyComponent
        }
    }

    ColumnLayout {
        id: dayForecast
        Layout.fillWidth: true
        visible: root.showDayForecast

        spacing: Kirigami.Units.mediumSpacing

        Kirigami.Heading {
            Layout.fillWidth: true
            level: 3
            visible: root.showDayForecast
            text: i18nc("@title %1 is a number of days", "%1-Day Forecast", stackedDayLoader.item?.daysCount ?? 0)
        }

        Loader {
            id: stackedDayLoader
            Layout.fillWidth: true
            visible: status == Loader.Ready
            active: root.showDayForecast
            sourceComponent: root.dayComponent
        }
    }

    Loader {
        id: sourceLabelLoader
        Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
        visible: status == Loader.Ready
        sourceComponent: root.sourceLabelComponent
    }
}
