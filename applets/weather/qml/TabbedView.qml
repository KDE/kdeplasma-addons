import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

ColumnLayout {
    id: root

    property bool showHourlyForecast: false
    property string hourlySubHeaderText: ""
    property var hourlyComponent: null

    property int forecastDaysNumber: 0
    property var dayComponent: null
    property bool showDayForecast: false

    property alias hourlyItem: tabbedHourlyLoader.item
    property alias dayItem: tabbedDayLoader.item

    PlasmaComponents.TabBar {
        id: tabBar

        Layout.fillWidth: true

        visible: root.showHourlyForecast && root.showDayForecast

        PlasmaComponents.TabButton {
            contentItem: ColumnLayout {
                spacing: 0
                PlasmaComponents.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: i18n("Hourly")
                }

                PlasmaComponents.Label {
                    Layout.alignment: Qt.AlignHCenter
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    font.family: Kirigami.Theme.smallFont.family
                    opacity: 0.75
                    text: root.hourlySubHeaderText
                }
            }
        }

        PlasmaComponents.TabButton {
            text: i18np("%1 Day", "%1 Days", root.forecastDaysNumber)
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
            active: root.showHourlyForecast
            visible: status == Loader.Ready
            sourceComponent: root.hourlyComponent
        }

        Loader {
            id: tabbedDayLoader
            active: root.showDayForecast
            visible: status == Loader.Ready
            sourceComponent: root.dayComponent
        }

        onCurrentIndexChanged: {
            tabBar.setCurrentIndex(currentIndex);
        }
    }
}
