/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
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

import QtQuick 2.1

import org.kde.plasma.plasmoid 2.0

Item {
    id: root

    property string currentWeatherIconName: {
        var panelModel = plasmoid.nativeInterface.panelModel;
        return !panelModel.location ? "weather-none-available" : panelModel.currentConditionIcon;
    }

    Plasmoid.icon: currentWeatherIconName
    Plasmoid.toolTipMainText: {
        // workaround for now to ensure "Please Configure" tooltip
        // TODO: remove when configurationRequired works
        return plasmoid.nativeInterface.panelModel.location || i18nc("Shown when you have not set a weather provider", "Please Configure");
    }
    Plasmoid.toolTipSubText: {
        var panelModel = plasmoid.nativeInterface.panelModel;
        if (!panelModel.location) {
            return "";
        }
        if (panelModel.currentConditions && panelModel.currentTemperature) {
            return i18nc("%1 is the weather condition, %2 is the temperature,  both come from the weather provider",
                         "%1 %2", panelModel.currentConditions, panelModel.currentTemperature);
        }
        return panelModel.currentConditions || panelModel.currentTemperature || "";
    }

    Plasmoid.compactRepresentation: CompactRepresentation {
    }

    Plasmoid.fullRepresentation: FullRepresentation {
    }

    Component.onCompleted: {
        // workaround for missing note about being in systray or similar (kde bug #388995)
        // guess from cointainer structure data and make available to config page
        plasmoid.nativeInterface.needsToBeSquare =
            (plasmoid.parent !== null &&
            ((plasmoid.parent.pluginName === 'org.kde.plasma.private.systemtray' ||
              plasmoid.parent.objectName === 'taskItemContainer')));
    }
}
