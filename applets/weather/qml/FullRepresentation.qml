/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

PlasmaExtras.Representation {
    id: root

    property int status: 0

    property int invalidUnit: 0
    property int displaySpeedUnit: 0
    property int displayPressureUnit: 0
    property int displayTemperatureUnit: 0
    property int displayVisibilityUnit: 0

    property var station: null
    property var futureHours: null
    property var futureDays: null
    property var warnings: null
    property var lastObservation: null
    property var metaData: null

    Layout.minimumWidth: Math.max(Kirigami.Units.gridUnit * 25, Math.max(Kirigami.Units.gridUnit * 10, stack.implicitWidth))
    Layout.minimumHeight: Math.min(Kirigami.Units.gridUnit * 10, stack.implicitHeight)
    Layout.margins: Kirigami.Units.smallSpacing

    header: PlasmaExtras.PlasmoidHeading {
        visible: stack.depth > 1
        contentItem: RowLayout {
            PlasmaComponents.ToolButton {
                icon.name: "go-previous"
                onClicked: stack.removePage(stack.lastItem)
            }
        }
    }

    PlasmaExtras.PlaceholderMessage {
        id: placeholderLocation
        anchors.centerIn: parent
        anchors.margins: Kirigami.Units.gridUnit
        // when not in panel, a configure button is already shown for needsConfiguration
        visible: (root.status === ForecastControl.NeedsConfiguration) && (Plasmoid.formFactor === PlasmaCore.Types.Vertical || Plasmoid.formFactor === PlasmaCore.Types.Horizontal)
        iconName: "mark-location"
        text: i18n("Please set your location")
        helpfulAction: QQC2.Action {
            icon.name: "configure"
            text: i18n("Set location…")

            onTriggered: {
                Plasmoid.internalAction("configure").trigger();
            }
        }
    }

    PlasmaExtras.PlaceholderMessage {
        anchors.centerIn: parent
        anchors.margins: Kirigami.Units.largeSpacing * 4
        width: Math.min(parent.width, Kirigami.Units.gridUnit * 20)
        visible: root.status === ForecastControl.Timeout
        iconName: "network-disconnect"
        text: !!root.station?.place ? i18n("Unable to retrieve weather information for %1", root.station.place) : i18n("Unable to retrieve weather information")
        explanation: i18nc("@info:usagetip", "The network request timed out, possibly due to a server outage at the weather station provider. Check again later.")
    }

    QQC2.BusyIndicator {
        id: busy
        anchors.centerIn: parent
        visible: root.status === ForecastControl.Connecting
    }

    Kirigami.PageRow {
        id: stack

        anchors.fill: parent

        initialPage: weatherPage

        defaultColumnWidth: currentItem.width

        WeatherPage {
            id: weatherPage

            status: root.status

            invalidUnit: root.invalidUnit
            displaySpeedUnit: root.displaySpeedUnit
            displayPressureUnit: root.displayPressureUnit
            displayTemperatureUnit: root.displayTemperatureUnit
            displayVisibilityUnit: root.displayVisibilityUnit

            station: root.station
            futureHours: root.futureHours
            futureDays: root.futureDays
            warnings: root.warnings
            lastObservation: root.lastObservation
            metaData: root.metaData

            onOpenWarnings: stack.push(warningsPage)
        }

        WarningsPage {
            id: warningsPage

            model: root.warnings
        }
    }
}
