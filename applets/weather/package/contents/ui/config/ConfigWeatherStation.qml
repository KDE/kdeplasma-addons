/*
 * Copyright 2016,2018  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Controls 2.5 as QtControls
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.private.weather 1.0


Kirigami.FormLayout {
    id: weatherStationConfigPage

    property string source

    signal configurationChanged

    function saveConfig() {
        var config = {
            services: stationPicker.selectedServices,
            source: source,
            updateInterval: updateIntervalSpin.value
        };

        plasmoid.nativeInterface.saveConfig(config);
        plasmoid.nativeInterface.configChanged();
    }


    Component.onCompleted: {
        var config = plasmoid.nativeInterface.configValues();

        stationPicker.selectedServices = config.services;

        source = config.source;

        updateIntervalSpin.value = config.updateInterval;
    }

    WeatherStationPickerDialog {
        id: stationPicker

        onAccepted: {
            weatherStationConfigPage.source = source;
            weatherStationConfigPage.configurationChanged();
        }
    }


    RowLayout {
        Kirigami.FormData.label: i18nc("@label", "Location:")
        Layout.fillWidth: true

        QtControls.Label {
            id: locationDisplay
            Layout.fillWidth: true
            elide: Text.ElideRight
            visible: text != ""

            text: {
                var sourceDetails = source.split('|');
                if (sourceDetails.length > 2) {
                    return i18nc("A weather station location and the weather service it comes from",
                                    "%1 (%2)", sourceDetails[2], sourceDetails[0]);
                }
                return ""
            }
        }

        QtControls.Button {
            id: selectButton
            Layout.fillWidth: true
            icon.name: "find-location"
            text: i18nc("@action:button", "Choose...")
            onClicked: stationPicker.visible = true;
        }
    }

    QtControls.SpinBox {
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

        onValueChanged: weatherStationConfigPage.configurationChanged();
    }
}
