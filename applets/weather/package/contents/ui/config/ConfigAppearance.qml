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
import QtQuick.Controls 1.4 as QtControls1
import QtQuick.Controls 2.2 as QtControls
import QtQuick.Layouts 1.3

import org.kde.plasma.private.weather 1.0


ColumnLayout {
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


    QtControls1.GroupBox {
        Layout.fillWidth: true
        flat: true

        title: i18nc("@title:group", "Compact Mode")

        GridLayout {
            columns: 2

            QtControls.Label {
                Layout.row: 0
                Layout.column: 0
                Layout.alignment: Qt.AlignRight
                enabled: canShowMoreInCompactMode
                text: i18nc("@label", "Display:")
            }

            QtControls.CheckBox {
                id: showTemperatureInCompactModeCheckBox

                Layout.row: 0
                Layout.column: 1
                enabled: canShowMoreInCompactMode

                text: i18nc("@option:check", "Show temperature")
                onCheckedChanged: displayConfigPage.configurationChanged();
            }

            QtControls.Label {
                Layout.row: 1
                Layout.column: 0
                Layout.alignment: Qt.AlignRight
                text: i18nc("@label", "Tooltip:")
            }

            QtControls.CheckBox {
                id: showTemperatureInTooltipCheckBox

                Layout.row: 1
                Layout.column: 1

                text: i18nc("@option:check", "Show temperature")
                onCheckedChanged: displayConfigPage.configurationChanged();
            }

            QtControls.CheckBox {
                id: showWindInTooltipCheckBox

                Layout.row: 2
                Layout.column: 1

                text: i18nc("@option:check", "Show wind")
                onCheckedChanged: displayConfigPage.configurationChanged();
            }

            QtControls.CheckBox {
                id: showPressureInTooltipCheckBox

                Layout.row: 3
                Layout.column: 1

                text: i18nc("@option:check", "Show pressure")
                onCheckedChanged: displayConfigPage.configurationChanged();
            }

            QtControls.CheckBox {
                id: showHumidityInTooltipCheckBox

                Layout.row: 4
                Layout.column: 1

                text: i18nc("@option:check", "Show humidity")
                onCheckedChanged: displayConfigPage.configurationChanged();
            }
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
