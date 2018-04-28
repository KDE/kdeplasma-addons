/*
 * Copyright (C) 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

ColumnLayout {
    id: generalSettings

    property alias cfg_cpuActivated: cpuActivatedCheckBox.checked
    property alias cfg_cpuAllActivated: cpuAllActivatedCheckBox.checked
    property alias cfg_memoryActivated: memoryActivatedCheckBox.checked
    property alias cfg_swapActivated: swapActivatedCheckBox.checked
    property alias cfg_cacheActivated: cacheActivatedCheckBox.checked
    property alias cfg_updateInterval: updateIntervalSpinBox.value
    property int cfg_monitorType: plasmoid.configuration.monitorType

    onCfg_monitorTypeChanged: {
        switch (cfg_monitorType) {
            default: case 0: monitorTypeGroup.current = barMonitorRadio; break;
            case 1: monitorTypeGroup.current = circularMonitorRadio; break;
            case 2: monitorTypeGroup.current = compactBarMonitorRadio; break;
        }
    }

    ExclusiveGroup {
        id: monitorTypeGroup
    }

    GridLayout {
        columns: 2

        Label {
            Layout.row: 0
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label", "Show:")
        }

        CheckBox {
            id: cpuActivatedCheckBox
            Layout.row: 0
            Layout.column: 1
            text: i18nc("@option:check", "CPU monitor")
            onCheckedChanged: if (!checked) {cpuAllActivatedCheckBox.checked = false;}
        }

        Row {
            Layout.row: 1
            Layout.column: 1
            Item { height: 1; width: 50; }
            CheckBox {
                id: cpuAllActivatedCheckBox
                text: i18nc("@option:check", "CPUs separately")
                enabled: cpuActivatedCheckBox.checked && cfg_monitorType === 2
            }
        }

        CheckBox {
            id: memoryActivatedCheckBox
            Layout.row: 2
            Layout.column: 1
            text: i18nc("@option:check", "Memory monitor")
        }

        CheckBox {
            id: swapActivatedCheckBox
            Layout.row: 3
            Layout.column: 1
            text: i18nc("@option:check", "Swap monitor")
        }

        CheckBox {
            id: cacheActivatedCheckBox
            Layout.row: 4
            Layout.column: 1
            text: i18nc("@option:check", "Cache monitor")
        }

        Label {
            Layout.row: 5
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label", "Monitor type:")
        }

        RadioButton {
            id: barMonitorRadio
            Layout.row: 5
            Layout.column: 1
            exclusiveGroup: monitorTypeGroup
            text: i18nc("@option:radio", "Bar")
            onCheckedChanged: if (checked) {cfg_monitorType = 0; cpuAllActivatedCheckBox.checked = false;}
        }

        RadioButton {
            id: circularMonitorRadio
            Layout.row: 6
            Layout.column: 1
            exclusiveGroup: monitorTypeGroup
            text: i18nc("@option:radio", "Circular")
            onCheckedChanged: if (checked) {cfg_monitorType = 1; cpuAllActivatedCheckBox.checked = false;}
        }

        RadioButton {
            id: compactBarMonitorRadio
            Layout.row: 7
            Layout.column: 1
            exclusiveGroup: monitorTypeGroup
            text: i18nc("@option:radio", "Compact bar")
            onCheckedChanged: if (checked) cfg_monitorType = 2;
        }

        Label {
            Layout.row: 8
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:spinbox", "Update interval:")
        }

        SpinBox {
            id: updateIntervalSpinBox
            Layout.row: 8
            Layout.column: 1
            decimals: 1
            stepSize: 0.1
            minimumValue: 0.1
            suffix: i18nc("@item:valuesuffix spacing to number + unit (seconds)", " s")
        }
    }


    Item { // tighten layout
        Layout.fillHeight: true
    }

    Component.onCompleted: cfg_monitorTypeChanged()
}
