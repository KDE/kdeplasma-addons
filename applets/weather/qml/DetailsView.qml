/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

RowLayout {
    id: root

    property var lastObservation: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0
    property int displayVisibilityUnit: 0
    property int displayPressureUnit: 0
    property int displaySpeedUnit: 0

    spacing: Kirigami.Units.smallSpacing

    component DetailsString: QtObject {
        property string label
        property string text
        property bool available: true
    }

    readonly property list<DetailsString> detailsModel: [
        DetailsString {
            label: i18nc("@label ground temperature", "Dewpoint")
            text: available ? Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lastObservation.dewpoint, root.metaData.temperatureUnit) : ""
            available: !!root.lastObservation?.dewpoint && !!root.metaData?.temperatureUnit
        },
        DetailsString {
            label: i18nc("@label", "Pressure")
            text: available ? Util.valueToDisplayString(root.displayPressureUnit, root.lastObservation.pressure, root.metaData.pressureUnit, 2) : ""
            available: !!root.lastObservation?.pressure && !!root.metaData?.pressureUnit
        },
        DetailsString {
            label: i18nc("@label pressure tendency, rising/falling/steady", "Pressure tendency")
            text: available ? root.lastObservation.pressureTendency : ""
            available: !!root.lastObservation?.pressureTendency
        },
        DetailsString {
            label: i18nc("@label", "Visibility")
            text: {
                if (!available) {
                    return "";
                }
                if (typeof root.lastObservation.visibility === "string") {
                    return root.lastObservation.visibility;
                }
                return Util.valueToDisplayString(root.displayVisibilityUnit, root.lastObservation.visibility, root.metaData.visibilityUnit, 1);
            }
            available: !!root.lastObservation?.visibility && (!!root.metaData?.visibilityUnit || typeof root.lastObservation.visibility === "string")
        },
        DetailsString {
            label: i18nc("@label", "Humidity")
            text: available ? Util.percentToDisplayString(root.lastObservation.humidity) : ""
            available: !!root.lastObservation?.humidity && !!root.metaData?.humidityUnit
        },
        DetailsString {
            label: i18nc("@label", "Wind gust")
            text: available ? Util.valueToDisplayString(root.displaySpeedUnit, root.lastObservation.windGust, root.metaData.windSpeedUnit, 1) : ""
            available: !!root.lastObservation?.windGust && !!root.metaData?.windSpeedUnit
        },
        DetailsString {
            label: i18nc("@label", "Wind speed")
            text: available ? Util.valueToDisplayString(root.displaySpeedUnit, root.lastObservation.windSpeed, root.metaData.windSpeedUnit, 1) : ""
            available: !isNaN(root.lastObservation?.windSpeed) && root.lastObservation.windSpeed !== 0.0 && !!root.metaData?.windSpeedUnit
        }
    ]

    readonly property list<DetailsString> detailsVisibleModel: detailsModel.filter(page => page.available)

    ColumnLayout {
        // Use this spacing because Kirigami.Units.smallSpacing is too large and
        // causes the applet to exceed its minimum size, showing a scrollbar.
        spacing: 1
        Repeater {
            model: root.detailsVisibleModel

            delegate: PlasmaComponents.Label {
                required property DetailsString modelData

                Layout.alignment: Qt.AlignRight

                text: modelData.label + ":"
                font: Kirigami.Theme.smallFont
                opacity: 0.75
            }
        }
    }

    ColumnLayout {
        // Use this spacing because Kirigami.Units.smallSpacing is too large and
        // causes the applet to exceed its minimum size, showing a scrollbar.
        spacing: 1
        Repeater {
            model: root.detailsVisibleModel

            delegate: PlasmaComponents.Label {
                required property DetailsString modelData

                text: modelData.text
                font: Kirigami.Theme.smallFont
            }
        }
    }
}
