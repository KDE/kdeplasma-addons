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

import QtQuick 2.5
import QtQuick.Controls 2.5 as QtControls

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.private.weather 1.0


Kirigami.FormLayout {
    id: unitsConfigPage

    signal configurationChanged

    function saveConfig() {
        var config = {};

        config.temperatureUnitId =
            TemperatureUnitListModel.unitIdForListIndex(temperatureComboBox.currentIndex);
        config.pressureUnitId =
            PressureUnitListModel.unitIdForListIndex(pressureComboBox.currentIndex);
        config.windSpeedUnitId =
            WindSpeedUnitListModel.unitIdForListIndex(windSpeedComboBox.currentIndex);
        config.visibilityUnitId =
            VisibilityUnitListModel.unitIdForListIndex(visibilityComboBox.currentIndex);

        plasmoid.nativeInterface.saveConfig(config);
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        var config = plasmoid.nativeInterface.configValues();

        temperatureComboBox.currentIndex =
            TemperatureUnitListModel.listIndexForUnitId(config.temperatureUnitId);
        pressureComboBox.currentIndex =
            PressureUnitListModel.listIndexForUnitId(config.pressureUnitId);
        windSpeedComboBox.currentIndex =
            WindSpeedUnitListModel.listIndexForUnitId(config.windSpeedUnitId);
        visibilityComboBox.currentIndex =
            VisibilityUnitListModel.listIndexForUnitId(config.visibilityUnitId);
    }

    QtControls.ComboBox {
        id: temperatureComboBox

        Kirigami.FormData.label: i18nc("@label:listbox", "Temperature:")

        model: TemperatureUnitListModel
        textRole: "display"
        onCurrentIndexChanged: unitsConfigPage.configurationChanged();
    }

    QtControls.ComboBox {
        id: pressureComboBox

        Kirigami.FormData.label: i18nc("@label:listbox", "Pressure:")

        model: PressureUnitListModel
        textRole: "display"
        onCurrentIndexChanged: unitsConfigPage.configurationChanged();
    }

    QtControls.ComboBox {
        id: windSpeedComboBox

        Kirigami.FormData.label: i18nc("@label:listbox", "Wind speed:")

        model: WindSpeedUnitListModel
        textRole: "display"
        onCurrentIndexChanged: unitsConfigPage.configurationChanged();
    }

    QtControls.ComboBox {
        id: visibilityComboBox

        Kirigami.FormData.label: i18nc("@label:listbox", "Visibility:")

        model: VisibilityUnitListModel
        textRole: "display"
        onCurrentIndexChanged: unitsConfigPage.configurationChanged();
    }
}
