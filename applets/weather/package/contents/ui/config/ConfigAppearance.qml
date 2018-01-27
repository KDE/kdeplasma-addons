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

import QtQuick 2.0
import QtQuick.Controls 2.2 as QtControls
import QtQuick.Layouts 1.3

import org.kde.plasma.private.weather 1.0


ColumnLayout {
    id: displayConfigPage

    readonly property bool canShowTemperature: !plasmoid.nativeInterface.needsToBeSquare

    signal configurationChanged

    function saveConfig() {
        var config = {};

        config.showTemperatureInCompactMode = showTemperatureCheckBox.checked;

        plasmoid.nativeInterface.saveConfig(config);
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        var config = plasmoid.nativeInterface.configValues();

        showTemperatureCheckBox.checked = config.showTemperatureInCompactMode;
    }


    GridLayout {
        columns: 2

        QtControls.Label {
            Layout.row: 0
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            enabled: canShowTemperature
            text: i18n("Show temperature in compact mode:")
        }

        QtControls.CheckBox {
            id: showTemperatureCheckBox

            Layout.row: 0
            Layout.column: 1
            enabled: canShowTemperature

            onCheckedChanged: displayConfigPage.configurationChanged();
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
