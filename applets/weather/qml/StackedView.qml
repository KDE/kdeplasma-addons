import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

Item {
    id: root

    property bool showHourlyForecast: false
    property string hourlySubHeaderText: ""
    property var hourlyComponent: null

    property int forecastDaysNumber: 0
    property var dayComponent: null
    property bool showDayForecast: false

    property alias hourlyItem: stackedHourlyLoader.item
    property alias dayItem: stackedDayLoader.item

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

                Kirigami.Heading {
                    level: 3
                    text: i18n("Hourly Forecast")
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

            Kirigami.Heading {
                Layout.fillWidth: true
                level: 3
                visible: stackedDayLoader.active
                text: i18n("%1 Day Forecast", root.forecastDaysNumber)
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
