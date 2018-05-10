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

import QtQuick.Controls 1.4 as QtControls
import QtQuick.Layouts 1.3

import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.private.weather 1.0


ColumnLayout {
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

    GridLayout {
        columns: 2

        QtControls.Label {
            Layout.row: 0
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label", "Location:")
        }

        RowLayout {
            Layout.row: 0
            Layout.column: 1
            Layout.fillWidth: true

            QtControls.Label {
                id: locationDisplay
                Layout.fillWidth: true
                elide: Text.ElideRight

                text: {
                    var sourceDetails = source.split('|');
                    if (sourceDetails.length > 2) {
                        return i18nc("A weather station location and the weather service it comes from",
                                     "%1 (%2)", sourceDetails[2], sourceDetails[0]);
                    }
                    return i18nc("no weather station", "-");
                }
            }

            QtControls.Button {
                id: selectButton
                iconName: "edit-find"
                text: i18nc("@action:button", "Select")
                onClicked: stationPicker.visible = true;
            }
        }


        QtControls.Label {
            Layout.row: 1
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:spinbox", "Update every:")
        }
        QtControls.SpinBox {
            id: updateIntervalSpin
            Layout.row: 1
            Layout.column: 1
            Layout.minimumWidth: units.gridUnit * 8
            suffix: i18nc("@item:valuesuffix spacing to number + unit (minutes)", " min")
            stepSize: 5
            minimumValue: 30
            maximumValue: 3600
            onValueChanged: weatherStationConfigPage.configurationChanged();
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
