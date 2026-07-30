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

    implicitWidth: Math.max(stackedDayLoader.item?.implicitWidth ?? 0, stackedHourlyLoader.item?.implicitWidth ?? 0)
    implicitHeight: topPanel.implicitHeight + tabbedView.implicitHeight + sourceLabel.implicitHeight + weather.spacing * (weather.children.length - 2)

    topPadding: 0
    bottomPadding: 0
    rightPadding: Kirigami.Units.smallSpacing
    leftPadding: Kirigami.Units.smallSpacing

    readonly property bool hasHourlyForecast: !!futureHours && futureHours.hoursNumber > 0 && !!futureHoursPoints

    readonly property bool hasDayForecast: !!futureDays && futureDays.daysNumber > 0 && !!futureDaysPoints

    readonly property bool useTabs: {
        if (!stackedDayLoader.item || !stackedHourlyLoader.item) {
            return false;
        }
        const requiredHeight = topPanel.implicitHeight + stackedView.implicitHeight + sourceLabel.implicitHeight + weather.spacing * (weather.children.length - 2);

        return requiredHeight > availableHeight;
    }

    ColumnLayout {
        id: weather

        anchors.fill: parent

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

        ColumnLayout {
            id: stackedView

            visible: !root.useTabs

            Layout.fillWidth: true
            Layout.fillHeight: true

            Kirigami.Heading {
                Layout.fillWidth: true
                level: 3
                visible: stackedHourlyLoader.active
                text: {
                    let defaultLabel = i18n("Hourly Forecast");

                    if (!stackedHourlyLoader.item || stackedHourlyLoader.item.currentIndex <= 0 || !root.futureHours) {
                        return defaultLabel;
                    }

                    const index = root.futureHours.index(stackedHourlyLoader.item.currentIndex, 0);
                    const format = Qt.locale().dateFormat(Locale.ShortFormat);
                    const date = Qt.formatDateTime(root.futureHours.data(index, WeatherData.FutureHours.Timestamp), format);

                    const color = Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.75);

                    return defaultLabel + " <font color=\"" + color + "\">" + date + "</font>";
                }
            }

            Loader {
                id: stackedHourlyLoader
                Layout.fillWidth: true
                active: root.hasHourlyForecast
                sourceComponent: root.showHourlyTemperatureGraph ? hourlyForecastGraph : hourlyForecastView
            }

            Kirigami.Heading {
                Layout.fillWidth: true
                level: 3
                visible: stackedDayLoader.active
                text: i18n("%1 Day Forecast", root.futureDays?.daysNumber)
            }

            Loader {
                id: stackedDayLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
                active: root.hasDayForecast
                sourceComponent: root.showDayTemperatureGraph ? dayForecastGraph : dayForecastView
            }
        }

        ColumnLayout {
            id: tabbedView

            visible: root.useTabs

            Layout.fillWidth: true

            PlasmaComponents.TabBar {
                id: tabBar

                Layout.fillWidth: true

                visible: root.hasHourlyForecast && root.hasDayForecast

                PlasmaComponents.TabButton {
                    text: {
                        let defaultLabel = i18n("Hourly");

                        if (!tabbedHourlyLoader.item || tabbedHourlyLoader.item.currentIndex <= 0 || !root.futureHours) {
                            return defaultLabel;
                        }

                        const index = root.futureHours.index(tabbedHourlyLoader.item.currentIndex, 0);
                        const format = Qt.locale().dateFormat(Locale.ShortFormat);
                        const date = Qt.formatDateTime(root.futureHours.data(index, WeatherData.FutureHours.Timestamp), format);

                        const color = Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.75);

                        return defaultLabel + " <font color=\"" + color + "\">" + date + "</font>";
                    }
                }

                PlasmaComponents.TabButton {
                    text: i18np("%1 Day", "%1 Days", root.futureDays?.daysNumber ?? 0)
                }

                onCurrentIndexChanged: {
                    swipeView.setCurrentIndex(currentIndex);
                }
            }

            QQC2.SwipeView {
                id: swipeView

                Layout.fillWidth: true
                Layout.minimumWidth: contentChildren.reduce((acc, loader) => Math.max(loader.implicitWidth, acc), 0)
                Layout.minimumHeight: contentChildren.reduce((acc, loader) => Math.max(loader.implicitHeight, acc), 0)
                clip: true // previous/next views are prepared outside of view, do not render them

                currentIndex: tabBar.currentIndex

                Loader {
                    id: tabbedHourlyLoader
                    Layout.fillWidth: true
                    active: root.hasHourlyForecast
                    sourceComponent: root.showHourlyTemperatureGraph ? hourlyForecastGraph : hourlyForecastView
                }

                Loader {
                    id: tabbedDayLoader
                    Layout.fillWidth: true
                    active: root.hasDayForecast
                    sourceComponent: root.showDayTemperatureGraph ? dayForecastGraph : dayForecastView
                }

                onCurrentIndexChanged: {
                    tabBar.setCurrentIndex(currentIndex);
                }
            }
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
                Layout.alignment: Qt.AlignTop
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
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true

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
