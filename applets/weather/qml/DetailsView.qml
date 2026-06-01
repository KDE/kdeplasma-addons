/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

ColumnLayout {
    id: root

    property var lastObservation: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0
    property int displayVisibilityUnit: 0
    property int displayPressureUnit: 0
    property int displaySpeedUnit: 0

    component DetailsString: QtObject {
        property string label
        property string text
        property bool visible: true
    }

    readonly property list<DetailsString> detailsModel: [
        DetailsString {
            label: i18nc("@label ground temperature", "Dewpoint:")
            text: visible ? Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lastObservation.dewpoint, root.metaData.temperatureUnit) : ""
            visible: !!root.lastObservation?.dewpoint && !!root.metaData?.temperatureUnit
        },
        DetailsString {
            label: i18nc("@label", "Pressure:")
            text: visible ? Util.valueToDisplayString(root.displayPressureUnit, root.lastObservation.pressure, root.metaData.pressureUnit, 2) : ""
            visible: !!root.lastObservation?.pressure && !!root.metaData?.pressureUnit
        },
        DetailsString {
            label: i18nc("@label pressure tendency, rising/falling/steady", "Pressure Tendency:")
            text: visible ? root.lastObservation.pressureTendency : ""
            visible: !!root.lastObservation?.pressureTendency
        },
        DetailsString {
            label: i18nc("@label", "Visibility:")
            text: {
                if (!visible) {
                    return "";
                }
                if (typeof root.lastObservation.visibility === "string") {
                    return root.lastObservation.visibility;
                }
                return Util.valueToDisplayString(root.displayVisibilityUnit, root.lastObservation.visibility, root.metaData.visibilityUnit, 1);
            }
            visible: !!root.lastObservation?.visibility && (!!root.metaData?.visibilityUnit || typeof root.lastObservation.visibility === "string")
        },
        DetailsString {
            label: i18nc("@label", "Humidity:")
            text: visible ? Util.percentToDisplayString(root.lastObservation.humidity) : ""
            visible: !!root.lastObservation?.humidity && !!root.metaData?.humidityUnit
        },
        DetailsString {
            label: i18nc("@label", "Wind Gust:")
            text: visible ? Util.valueToDisplayString(root.displaySpeedUnit, root.lastObservation.windGust, root.metaData.windSpeedUnit, 1) : ""
            visible: !!root.lastObservation?.windGust && !!root.metaData?.windSpeedUnit
        },
        DetailsString {
            label: i18nc("@label", "Wind Speed:")
            text: visible ? Util.valueToDisplayString(root.displaySpeedUnit, root.lastObservation.windSpeed, root.metaData.windSpeedUnit, 1) : ""
            visible: !isNaN(root.lastObservation?.windSpeed) && root.lastObservation.windSpeed !== 0.0 && !!root.metaData?.windSpeedUnit
        }
    ]

    readonly property list<DetailsString> detailsVisibleModel: detailsModel.filter(page => page.visible)

    GridLayout {
        Layout.alignment: Qt.AlignCenter

        rows: labelRepeater.count
        flow: GridLayout.TopToBottom
        rowSpacing: Kirigami.Units.smallSpacing

        Repeater {
            id: labelRepeater
            model: root.detailsVisibleModel
            delegate: PlasmaComponents.Label {
                Layout.alignment: Qt.AlignRight
                text: model.label + model.text
                textFormat: Text.PlainText
            }
        }
    }
}
