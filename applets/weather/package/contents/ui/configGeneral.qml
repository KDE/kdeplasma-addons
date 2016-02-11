/*
 * Copyright 2016  Friedrich W. H. Kossebau <kossebau@kde.org>
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

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0

import org.kde.plasma.private.weather 1.0

ColumnLayout {
    id: generalConfigPage

    signal configurationChanged

    function saveConfig() {
        plasmoid.nativeInterface.updateInterval = updateIntervalSpin.value;

        plasmoid.nativeInterface.temperatureUnitId =
            TemperatureUnitListModel.unitIdForListIndex(temperatureComboBox.currentIndex);
        plasmoid.nativeInterface.pressureUnitId =
            PressureUnitListModel.unitIdForListIndex(pressureComboBox.currentIndex);
        plasmoid.nativeInterface.windSpeedUnitId =
            WindSpeedUnitListModel.unitIdForListIndex(windSpeedComboBox.currentIndex);
        plasmoid.nativeInterface.visibilityUnitId =
            VisibilityUnitListModel.unitIdForListIndex(visibilityComboBox.currentIndex);

        plasmoid.nativeInterface.configAccepted();//comic uses saveConfig(); think about passing a config object here
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        updateIntervalSpin.value = plasmoid.nativeInterface.updateInterval;
        temperatureComboBox.currentIndex =
            TemperatureUnitListModel.listIndexForUnitId(plasmoid.nativeInterface.temperatureUnitId);
        pressureComboBox.currentIndex =
            PressureUnitListModel.listIndexForUnitId(plasmoid.nativeInterface.pressureUnitId);
        windSpeedComboBox.currentIndex =
            WindSpeedUnitListModel.listIndexForUnitId(plasmoid.nativeInterface.windSpeedUnitId);
        visibilityComboBox.currentIndex =
            VisibilityUnitListModel.listIndexForUnitId(plasmoid.nativeInterface.visibilityUnitId);
    }


    QtControls.GroupBox {
        title: i18n("Weather Station")
        flat: true

        RowLayout {
            Layout.alignment: Qt.AlignRight
            QtControls.Label {
                text: i18n("Update every:")
            }
            QtControls.SpinBox {
                id: updateIntervalSpin
                Layout.minimumWidth: units.gridUnit * 8
                suffix: i18n(" min")
                stepSize: 5
                minimumValue: 30
                maximumValue: 3600
                onValueChanged: generalConfigPage.configurationChanged();
            }
        }
    }

    QtControls.GroupBox {
        title: i18n("Units")
        flat: true

        GridLayout {
            Layout.fillWidth: false // Layout thinks it's smart whereas it's not
            Layout.alignment: Qt.AlignLeft
            columns: 2

            QtControls.Label {
                Layout.row: 0
                Layout.column: 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18n("Temperature:")
            }

            QtControls.ComboBox {
                id: temperatureComboBox
                Layout.row: 0
                Layout.column: 1
                model: TemperatureUnitListModel
                textRole: "display"
                onCurrentIndexChanged: generalConfigPage.configurationChanged();
            }

            QtControls.Label {
                Layout.row: 1
                Layout.column: 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18n("Pressure:")
            }

            QtControls.ComboBox {
                id: pressureComboBox
                Layout.row: 1
                Layout.column: 1
                model: PressureUnitListModel
                textRole: "display"
                onCurrentIndexChanged: generalConfigPage.configurationChanged();
            }

            QtControls.Label {
                Layout.row: 2
                Layout.column: 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18n("Wind speed:")
            }

            QtControls.ComboBox {
                id: windSpeedComboBox
                Layout.row: 2
                Layout.column: 1
                model: WindSpeedUnitListModel
                textRole: "display"
                onCurrentIndexChanged: generalConfigPage.configurationChanged();
            }

            QtControls.Label {
                Layout.row: 3
                Layout.column: 0
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18n("Visibility:")
            }

            QtControls.ComboBox {
                id: visibilityComboBox
                Layout.row: 3
                Layout.column: 1
                model: VisibilityUnitListModel
                textRole: "display"
                onCurrentIndexChanged: generalConfigPage.configurationChanged();
            }
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
