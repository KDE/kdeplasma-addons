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

ColumnLayout {
    id: root

    property int status: 0

    property int invalidUnit: 0
    property int displaySpeedUnit: 0
    property int displayPressureUnit: 0
    property int displayTemperatureUnit: 0
    property int displayVisibilityUnit: 0

    property var station: null
    property var futureDays: null
    property var warnings: null
    property var lastObservation: null
    property var metaData: null

    Layout.minimumWidth: Math.min(Kirigami.Units.gridUnit * 25,
        Math.max(Kirigami.Units.gridUnit * 10,
            implicitWidth))
    Layout.minimumHeight: Math.max(Kirigami.Units.gridUnit * 10, implicitHeight)

    Layout.margins: Kirigami.Units.smallSpacing

    PlasmaExtras.PlaceholderMessage {
        id: placeholderLocation
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: Kirigami.Units.gridUnit
        // when not in panel, a configure button is already shown for needsConfiguration
        visible: (root.status === ForecastControl.NeedsConfiguration) && (Plasmoid.formFactor === PlasmaCore.Types.Vertical || Plasmoid.formFactor === PlasmaCore.Types.Horizontal)
        iconName: "mark-location"
        text: i18n("Please set your location")
        helpfulAction: QQC2.Action
        {
            icon.name: "configure"
            text: i18n("Set location…")

            onTriggered: {
                Plasmoid.internalAction("configure").trigger();
            }
        }
    }

    PlasmaExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: Kirigami.Units.largeSpacing * 4
        Layout.maximumWidth: Kirigami.Units.gridUnit * 20
        visible: root.status === ForecastControl.Timeout
        iconName: "network-disconnect"
        text: !!root.station?.place ? i18n("Unable to retrieve weather information for %1", root.station.place) : i18n("Unable to retrieve weather information")
        explanation: i18nc("@info:usagetip", "The network request timed out, possibly due to a server outage at the weather station provider. Check again later.")
    }

    TopPanel {
        id: topPanel
        visible: (!!root.station || !!root.lastObservation) && root.status === ForecastControl.Normal

        station: root.station
        lastObservation: root.lastObservation
        metaData: root.metaData
        futureDays: root.futureDays

        displayWindSpeedUnit: root.displaySpeedUnit
        displayTemperatureUnit: root.displayTemperatureUnit

        Layout.fillWidth: true
        // Allow the top panel to vertically grow but within a limit
        Layout.fillHeight: true
        Layout.maximumHeight: implicitHeight * 1.5
    }

    SwitchPanel {
        id: switchPanel
        visible: root.status === ForecastControl.Normal
        Layout.fillWidth: true

        futureDays: root.futureDays
        warnings: root.warnings
        lastObservation: root.lastObservation
        metaData: root.metaData

        invalidUnit: root.invalidUnit
        displayPressureUnit: root.displayPressureUnit
        displaySpeedUnit: root.displaySpeedUnit
        displayVisibilityUnit: root.displayVisibilityUnit
        displayTemperatureUnit: root.displayTemperatureUnit
    }

    PlasmaComponents.Label {
        id: sourceLabel
        visible: root.status === ForecastControl.Normal

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            cursorShape: !!metaData?.credit ? Qt.PointingHandCursor : Qt.ArrowCursor
        }

        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignRight
        font.pointSize: Kirigami.Theme.smallFont.pointSize
        linkColor : color
        opacity: 0.75
        textFormat: Text.StyledText

        text: {
            let result = "";
            if (!!metaData?.credit) {
                if (!!metaData.creditURL) {
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
