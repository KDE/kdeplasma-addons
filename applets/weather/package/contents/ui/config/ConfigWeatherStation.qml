/*
 * SPDX-FileCopyrightText: 2016, 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.weather 1.0


Kirigami.FormLayout {
    id: weatherStationConfigPage

    property string cfg_source
    property alias cfg_updateInterval: updateIntervalSpin.value

    property var providers: Plasmoid.nativeInterface.providers

    WeatherStationPickerDialog {
        id: stationPicker
        providers: Plasmoid.nativeInterface.providers

        onAccepted: {
            weatherStationConfigPage.cfg_source = source;
        }
    }


    RowLayout {
        Kirigami.FormData.label: i18nc("@label", "Location:")
        Layout.fillWidth: true

        QQC2.Label {
            id: locationDisplay
            Layout.fillWidth: true
            elide: Text.ElideRight
            visible: text != ""

            text: {
                const sourceDetails = cfg_source.split('|');
                if (sourceDetails.length > 2) {
                    return i18nc("A weather station location and the weather service it comes from",
                                    "%1 (%2)", sourceDetails[2], Plasmoid.nativeInterface.providers[sourceDetails[0]]);
                }
                return ""
            }
        }

        QQC2.Button {
            id: selectButton
            Layout.fillWidth: true
            icon.name: "find-location"
            text: i18nc("@action:button", "Choose…")
            onClicked: stationPicker.visible = true;
        }
    }

    QQC2.SpinBox {
        id: updateIntervalSpin

        Kirigami.FormData.label: i18nc("@label:spinbox", "Update every:")

        textFromValue: function(value) {
            return (i18np("%1 minute", "%1 minutes", value));
        }
        valueFromText: function(text) {
            return parseInt(text);
        }

        from: 30
        to: 3600
        editable: true
    }
}
