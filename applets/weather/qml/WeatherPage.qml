/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.plasma.weatherdata as WeatherData

Kirigami.ScrollablePage {
    id: root

    property int status: 0

    required property bool showHourlyTemperatureGraph
    required property bool showDayTemperatureGraph

    property int invalidUnit: 0
    property int displaySpeedUnit: 0
    property int displayPressureUnit: 0
    property int displayTemperatureUnit: 0
    property int displayVisibilityUnit: 0

    property var station: null
    property var futureHours: null
    property var futureHoursPoints: null
    property var futureDays: null
    property var futureDaysPoints: null
    property var warnings: null
    property var lastObservation: null
    property var metaData: null

    signal openWarnings

    background: null

    implicitWidth: Math.max(stackedView.dayItem?.implicitWidth ?? 0, stackedView.hourlyItem?.implicitWidth ?? 0, topPanel.implicitWidth) + rightPadding + leftPadding
    implicitHeight: topPanel.implicitHeight + ((!!stackedView.dayItem && !!stackedView.hourlyItem) ? tabbedView.implicitHeight : stackedView.implicitHeight) + sourceLabel.implicitHeight + weather.spacing * (weather.children.length - 2) + topPadding + bottomPadding

    topPadding: 0
    bottomPadding: 0
    rightPadding: Kirigami.Units.smallSpacing
    leftPadding: Kirigami.Units.smallSpacing

    readonly property bool hasHourlyForecast: !!futureHours && futureHours.hoursNumber > 0 && !!futureHoursPoints

    readonly property bool hasDayForecast: !!futureDays && futureDays.daysNumber > 0 && !!futureDaysPoints

    readonly property bool useTabs: {
        if (!stackedView.dayItem || !stackedView.hourlyItem) {
            return false;
        }
        const requiredHeight = topPanel.implicitHeight + stackedView.implicitHeight + sourceLabel.implicitHeight + weather.spacing * (weather.children.length - 2);

        return requiredHeight > availableHeight;
    }

    function hourlyForecastHeading(currentIndex, hoursPerDay, shortFormat) {
        if (!root.futureHours || !root.futureHoursPoints) {
            return "";
        }

        const firstIndex = currentIndex;
        if (firstIndex < 0 || firstIndex >= root.futureHours.rowCount()) {
            return "";
        }

        const lastIndex = Math.min(root.futureHours.rowCount() - 1, firstIndex + hoursPerDay - 1);

        const firstDate = root.futureHours.data(root.futureHours.index(firstIndex, 0), WeatherData.FutureHours.Timestamp);
        const lastDate = root.futureHours.data(root.futureHours.index(lastIndex, 0), WeatherData.FutureHours.Timestamp);

        if (!firstDate || !lastDate) {
            return "";
        }

        function isSameDay(a, b) {
            return a.getFullYear() === b.getFullYear() && a.getMonth() === b.getMonth() && a.getDate() === b.getDate();
        }

        function getDayName(date) {
            return Qt.locale().dayName(date.getDay(), !shortFormat ? Locale.LongFormat : Locale.ShortFormat);
        }

        const today = new Date();
        const tomorrow = new Date(today);
        tomorrow.setDate(today.getDate() + 1);

        if (isSameDay(firstDate, lastDate)) {
            if (isSameDay(firstDate, today)) {
                return i18nc("@label", "Today");
            } else if (isSameDay(firstDate, tomorrow)) {
                return i18nc("@label", "Tomorrow");
            }
            return getDayName(firstDate);
        } else {
            if (isSameDay(firstDate, today)) {
                if (isSameDay(lastDate, tomorrow)) {
                    return i18nc("@label", "Today to Tomorrow");
                } else {
                    return i18nc("@label %1 is a day of the week", "Today to %1", getDayName(lastDate));
                }
            }
            return i18nc("@label %1 and %2 are days of the week", getDayName(firstDate), getDayName(lastDate));
        }

        return "";
    }

    ColumnLayout {
        id: weather

        anchors.fill: parent

        spacing: 0

        TopPanel {
            id: topPanel
            visible: !!root.station || !!root.lastObservation

            station: root.station
            lastObservation: root.lastObservation
            metaData: root.metaData
            warnings: root.warnings
            futureDays: root.futureDays

            invalidUnit: root.invalidUnit
            displayPressureUnit: root.displayPressureUnit
            displaySpeedUnit: root.displaySpeedUnit
            displayVisibilityUnit: root.displayVisibilityUnit
            displayTemperatureUnit: root.displayTemperatureUnit

            Layout.alignment: Qt.AlignTop

            Layout.fillWidth: true

            onOpenWarnings: root.openWarnings()
        }

        StackedView {
            id: stackedView

            visible: !root.useTabs

            Layout.fillWidth: true
            Layout.fillHeight: true

            hourlySubHeaderText: !!hourlyItem ? root.hourlyForecastHeading(hourlyItem.currentIndex, hourlyItem.hoursPerPage) : ""
            showHourlyForecast: root.hasHourlyForecast
            hourlyComponent: root.showHourlyTemperatureGraph ? hourlyForecastGraph : hourlyForecastView

            showDayForecast: root.hasDayForecast
            dayComponent: root.showDayTemperatureGraph ? dayForecastGraph : dayForecastView
        }

        TabbedView {
            id: tabbedView

            visible: root.useTabs

            Layout.fillWidth: true

            showHourlyForecast: root.hasHourlyForecast
            hourlySubHeaderText: !!hourlyItem ? root.hourlyForecastHeading(hourlyItem.currentIndex, hourlyItem.hoursPerPage) : ""
            hourlyComponent: root.showHourlyTemperatureGraph ? hourlyForecastGraph : hourlyForecastView

            dayComponent: root.showDayTemperatureGraph ? dayForecastGraph : dayForecastView
            showDayForecast: root.hasDayForecast
        }

        Kirigami.UrlButton {
            id: sourceLabel

            Layout.alignment: Qt.AlignBottom | Qt.AlignLeft

            wrapMode: Text.WordWrap
            font.pointSize: Kirigami.Theme.smallFont.pointSize

            text: !!root.metaData?.credit ? root.metaData.credit : ""
            url: !!root.metaData?.creditURL ? root.metaData.creditURL : ""
        }

        Component {
            id: hourlyForecastView
            HourlyForecastCards {
                Layout.fillWidth: true

                futureHours: root.futureHours
                metaData: root.metaData

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
            }
        }

        Component {
            id: hourlyForecastGraph
            HourlyForecastGraph {
                Layout.fillWidth: true

                futureHoursPoints: root.futureHoursPoints
                metaData: root.metaData

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
            }
        }

        Component {
            id: dayForecastView
            DayForecastCards {
                Layout.fillWidth: true

                futureDays: root.futureDays
                metaData: root.metaData

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
            }
        }

        Component {
            id: dayForecastGraph
            DayForecastGraph {
                Layout.fillWidth: true

                futureDaysPoints: root.futureDaysPoints
                metaData: root.metaData

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
            }
        }
    }
}
