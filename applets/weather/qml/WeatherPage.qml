import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

Kirigami.ScrollablePage {
    id: root

    property int status: 0

    property bool showHourlyTemperatureGraph: false
    property bool showDayTemperatureGraph: false

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

    background: Rectangle {
        anchors.fill: parent
        color: "transparent"
    }

    implicitWidth: Math.max(stackedView.dayItem?.implicitWidth ?? 0, stackedView.hourlyItem?.implicitWidth ?? 0, topPanel.implicitWidth)
    implicitHeight: topPanel.implicitHeight + tabbedView.implicitHeight + sourceLabel.implicitHeight + weather.spacing * (weather.children.length - 2)

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

        function isSameDay(a, b) {
            return a.getFullYear() === b.getFullYear() && a.getMonth() === b.getMonth() && a.getDate() === b.getDate();
        }

        function getDayName(date) {
            return Qt.locale().dayName(date.getDay(), !shortFormat ? Locale.LongFormat : Locale.ShortFormat);
        }

        const today = new Date();
        const tomorrow = new Date(today);
        tomorrow.setDate(today.getDate() + 1);

        let period;

        if (isSameDay(firstDate, lastDate)) {
            if (isSameDay(firstDate, today)) {
                return i18nc("@label", "Today");
            } else if (isSameDay(firstDate, tomorrow)) {
                return i18nc("@label", "Tomorrow");
            } else {
                period = getDayName(firstDate);
            }
        } else {
            const firstText = isSameDay(firstDate, today) ? i18nc("@label", "Today") : getDayName(firstDate);

            const lastText = isSameDay(lastDate, tomorrow) ? i18nc("@label", "Tomorrow") : getDayName(lastDate);

            return i18nc("@label", "%1 to %2", firstText, lastText);
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

            hourlySubHeaderText: root.hourlyForecastHeading(hourlyItem.currentIndex, hourlyItem.hoursPerPage)
            showHourlyForecast: root.hasHourlyForecast
            hourlyComponent: root.showHourlyTemperatureGraph ? hourlyForecastGraph : hourlyForecastView

            forecastDaysNumber: root.futureDays?.daysNumber ?? 0
            showDayForecast: root.hasDayForecast
            dayComponent: root.showDayTemperatureGraph ? dayForecastGraph : dayForecastView
        }

        TabbedView {
            id: tabbedView

            visible: root.useTabs

            Layout.fillWidth: true

            showHourlyForecast: root.hasHourlyForecast
            hourlySubHeaderText: root.hourlyForecastHeading(tabbedView.hourlyItem.currentIndex, tabbedView.hourlyItem.hoursPerPage)
            hourlyComponent: root.showHourlyTemperatureGraph ? hourlyForecastGraph : hourlyForecastView

            forecastDaysNumber: root.futureDays?.daysNumber ?? 0
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
            HourlyForecastView {
                Layout.fillWidth: true
                Layout.fillHeight: true

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
                Layout.fillHeight: true

                futureHoursPoints: root.futureHoursPoints
                metaData: root.metaData

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
            }
        }

        Component {
            id: dayForecastView
            DayForecastView {
                Layout.fillWidth: true
                Layout.fillHeight: true

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
                Layout.fillHeight: true

                futureDaysPoints: root.futureDaysPoints
                metaData: root.metaData

                invalidUnit: root.invalidUnit
                displayTemperatureUnit: root.displayTemperatureUnit
            }
        }
    }
}
