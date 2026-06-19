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

    implicitWidth: weather.implicitWidth
    implicitHeight: weather.implicitHeight

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
            id: hourlyForecast

            Layout.alignment: Qt.AlignTop

            visible: !!root.futureHours && root.futureHours.hoursNumber > 0
            Kirigami.Heading {
                Layout.fillWidth: true
                level: 3
                text: i18n("Hourly Forecast")
            }

            Loader {
                id: hourlyLoader
                Layout.fillWidth: true
                sourceComponent: root.showHourlyTemperatureGraph ? hourlyForecastGraph : hourlyForecastView
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

                    futureHours: root.futureHours
                    futureHoursPoints: root.futureHoursPoints
                    metaData: root.metaData

                    invalidUnit: root.invalidUnit
                    displayTemperatureUnit: root.displayTemperatureUnit
                }
            }
        }

        ColumnLayout {
            id: dayForecast

            Layout.alignment: Qt.AlignTop

            visible: !!root.futureDays && root.futureDays.daysNumber > 0
            Kirigami.Heading {
                Layout.fillWidth: true
                level: 3
                text: i18n("%1 Day Forecast", root.futureDays?.daysNumber)
            }

            Loader {
                Layout.fillWidth: true
                sourceComponent: root.showDayTemperatureGraph ? dayForecastGraph : dayForecastView
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
                    futureDays: root.futureDays
                    futureDaysPoints: root.futureDaysPoints
                    metaData: root.metaData

                    invalidUnit: root.invalidUnit
                    displayTemperatureUnit: root.displayTemperatureUnit
                }
            }
        }

        PlasmaComponents.Label {
            id: sourceLabel

            Layout.alignment: Qt.AlignBottom | Qt.AlignLeft

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
                cursorShape: !!root.metaData?.credit ? Qt.PointingHandCursor : Qt.ArrowCursor
            }

            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignRight
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            linkColor: color
            opacity: 0.75
            textFormat: Text.StyledText

            text: {
                let result = "";
                if (!!root.metaData?.credit) {
                    if (!!root.metaData.creditURL) {
                        result = "<a href=\"" + root.metaData.creditURL + "\">" + root.metaData.credit + "</a>";
                    } else {
                        result = root.metaData.credit;
                    }
                }
                return result;
            }

            onLinkActivated: link => {
                Qt.openUrlExternally(link);
            }
        }
    }
}
