/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
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
    id: displayConfigPage

    readonly property bool canShowMoreInCompactMode: !plasmoid.nativeInterface.needsToBeSquare

    signal configurationChanged

    function saveConfig() {
        var config = {};

        config.showTemperatureInTooltip = showTemperatureInTooltipCheckBox.checked;
        config.showWindInTooltip = showWindInTooltipCheckBox.checked;
        config.showPressureInTooltip = showPressureInTooltipCheckBox.checked;
        config.showHumidityInTooltip = showHumidityInTooltipCheckBox.checked;

        config.showTemperatureInCompactMode = showTemperatureInCompactModeCheckBox.checked;

        plasmoid.nativeInterface.saveConfig(config);
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        var config = plasmoid.nativeInterface.configValues();

        showTemperatureInTooltipCheckBox.checked = config.showTemperatureInTooltip;
        showWindInTooltipCheckBox.checked = config.showWindInTooltip;
        showPressureInTooltipCheckBox.checked = config.showPressureInTooltip;
        showHumidityInTooltipCheckBox.checked = config.showHumidityInTooltip;

        showTemperatureInCompactModeCheckBox.checked = config.showTemperatureInCompactMode;
    }


    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Compact Mode")

        // TODO: conditionalize this once there are also settings for non-compact mode
        visible: false
    }

    QtControls.CheckBox {
        id: showTemperatureInCompactModeCheckBox

        Kirigami.FormData.label: i18nc("@label", "Show beside widget icon:")

        enabled: canShowMoreInCompactMode

        text: i18nc("@option:check Show on widget icon: temperature", "Temperature")
        onCheckedChanged: displayConfigPage.configurationChanged();
    }


    Item {
        Kirigami.FormData.isSection: true
    }


    QtControls.CheckBox {
        id: showTemperatureInTooltipCheckBox

        Kirigami.FormData.label: i18nc("@label", "Show in tooltip:")

        text: i18nc("@option:check", "Temperature")
        onCheckedChanged: displayConfigPage.configurationChanged();
    }

    QtControls.CheckBox {
        id: showWindInTooltipCheckBox

        text: i18nc("@option:check Show in tooltip: wind", "Wind")
        onCheckedChanged: displayConfigPage.configurationChanged();
    }

    QtControls.CheckBox {
        id: showPressureInTooltipCheckBox

        text: i18nc("@option:check Show in tooltip: pressure", "Pressure")
        onCheckedChanged: displayConfigPage.configurationChanged();
    }

    QtControls.CheckBox {
        id: showHumidityInTooltipCheckBox

        text: i18nc("@option:check Show in tooltip: humidity", "Humidity")
        onCheckedChanged: displayConfigPage.configurationChanged();
    }
}
