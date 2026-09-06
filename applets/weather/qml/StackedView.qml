/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Item {
    id: root

    property bool showHourlyForecast: false
    property string hourlySubHeaderText: ""
    property var hourlyComponent: null

    property var dayComponent: null
    property bool showDayForecast: false

    readonly property alias hourlyItem: stackedHourlyLoader.item
    readonly property alias dayItem: stackedDayLoader.item

    implicitHeight: (!!stackedHourlyLoader.item ? hourlyForecast.implicitHeight : 0) + (!!stackedDayLoader.item ? dayForecast.implicitHeight : 0)
    implicitWidth: forecast.implicitWidth

    ColumnLayout {
        id: forecast

        anchors.fill: parent

        // Center the forecast sections vertically by distributing the unused space
        // between the top margin, section spacing, and bottom margin.
        spacing: (root.height - root.implicitHeight) / (visibleChildren.length + 1)
        anchors.topMargin: spacing
        anchors.bottomMargin: spacing

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
                visible: stackedDayLoader.active
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
    }
}
