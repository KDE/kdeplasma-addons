/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras

ColumnLayout {
    id: root

    property var warnings: null
    property var futureDays: null
    property var futureHours: null
    property var lastObservation: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0
    property int displayVisibilityUnit: 0
    property int displayPressureUnit: 0
    property int displaySpeedUnit: 0

    property bool forecastViewNightRow: false

    property string forecastViewTitle: (!!futureDays && futureDays.daysNumber > 0) ? i18ncp("Forecast period timeframe", "1 Day", "%1 Days", futureDays.daysNumber) : ""

    component WeatherInfoPanel: QtObject {
        property string title
        property bool visible: true
        property Component view
        property string icon: ""
    }

    readonly property list<WeatherInfoPanel> weatherPanelModel: [
        WeatherInfoPanel {
            title: root.forecastViewTitle || i18nc("@title:tab Weather forecast", "Forecast")
            view: (!!root.futureDays && root.futureDays.daysNumber > 0) || (!!root.futureHours && root.futureHours.hoursNumber > 0) ? forecastView : forecastPlaceholder
        },
        WeatherInfoPanel {
            title: i18nc("@title:tab", "Details")
            visible: !!root.lastObservation && root.detailsVisibleModel.length > 0
            view: detailsView
        }
    ]

    readonly property list<WeatherInfoPanel> pagesModel: weatherPanelModel.filter(page => page.visible)

    PlasmaComponents.TabBar {
        id: tabBar

        Layout.fillWidth: true
        visible: root.pagesModel.length > 1
        implicitWidth: 0 // this avoids a binding loop

        Repeater {
            model: root.pagesModel
            delegate: PlasmaComponents.TabButton {
                text: modelData.title
                icon.name: modelData.icon ?? ""
            }
        }

        onCurrentIndexChanged: {
            // Avoid scrolling conflicts between SwipeView and NoticesView
            swipeView.interactive = false;
            swipeView.setCurrentIndex(currentIndex);
            swipeView.interactive = true;
        }
    }

    QQC2.SwipeView {
        id: swipeView

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.minimumWidth: contentChildren.reduce((acc, loader) => Math.max(loader.implicitWidth, acc), 0)
        Layout.minimumHeight: contentChildren.reduce((acc, loader) => Math.max(loader.implicitHeight, acc), 0)
        clip: true // previous/next views are prepared outside of view, do not render them

        Repeater {
            model: root.pagesModel
            delegate: Loader {
                sourceComponent: model.view
            }
        }

        onCurrentIndexChanged: {
            tabBar.setCurrentIndex(currentIndex);
        }
    }

    Component {
        id: forecastView
        ForecastView {
            futureHours: root.futureHours
            futureDays: root.futureDays
            metaData: root.metaData
            displayTemperatureUnit: root.displayTemperatureUnit
        }
    }

    Component {
        id: forecastPlaceholder
        ColumnLayout {
            PlasmaExtras.PlaceholderMessage {
                Layout.alignment: Qt.AlignCenter
                Layout.margins: Kirigami.Units.largeSpacing
                // Sets a minimum width for the placeholder tab
                Layout.preferredWidth: Kirigami.Units.gridUnit * 15

                text: i18nc("@info:placeholder", "Unable to load weather forecast")
                // TODO: Add a link to the bug-report url, which is now not possible to access within the placeholder
                explanation: i18nc("@info:usagetip", "There may be a technical issue with the weather provider. If the issue persists for longer than a day, submit a bug report.")
            }
        }
    }

    Component {
        id: detailsView
        DetailsView {
            invalidUnit: root.invalidUnit
            displayTemperatureUnit: root.displayTemperatureUnit
            displayVisibilityUnit: root.displayVisibilityUnit
            displayPressureUnit: root.displayPressureUnit
            displaySpeedUnit: root.displaySpeedUnit

            metaData: root.metaData
            lastObservation: root.lastObservation
        }
    }
}
