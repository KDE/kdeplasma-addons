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

Item {
    id: generalSettings

    property alias cfg_cpuActivated: cpuActivatedCheckBox.checked
    property alias cfg_cpuAllActivated: cpuAllActivatedCheckBox.checked
    property alias cfg_memoryActivated: memoryActivatedCheckBox.checked
    property alias cfg_swapActivated: swapActivatedCheckBox.checked
    property alias cfg_updateInterval: updateIntervalSpinBox.value
    property int cfg_monitorType: plasmoid.configuration.monitorType

    height: layout.implicitHeight
    width: layout.implicitWidth

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
        id: layout

        columns: 2

        Label {
            text: i18n("Show:")
            Layout.alignment: Qt.AlignRight
            Layout.rowSpan: 4
            anchors.verticalCenter: cpuActivatedCheckBox.verticalCenter
        }

        CheckBox {
            id: cpuActivatedCheckBox
            text: i18n("CPU monitor")
            onCheckedChanged: if (!checked) {cpuAllActivatedCheckBox.checked = false;}
        }

        Row {
            Item { height: 1; width: 50; }
            CheckBox {
                id: cpuAllActivatedCheckBox
                text: i18n("CPUs separately")
                enabled: cpuActivatedCheckBox.checked && cfg_monitorType === 2
            }
        }

        CheckBox {
            id: memoryActivatedCheckBox
            text: i18n("Memory monitor")
        }

        CheckBox {
            id: swapActivatedCheckBox
            text: i18n("Swap monitor")
        }

        Label {
            text: i18n("Monitor type:")
            Layout.alignment: Qt.AlignRight
            Layout.rowSpan: 3
            anchors.verticalCenter: barMonitorRadio.verticalCenter
        }

        RadioButton {
            id: barMonitorRadio
            exclusiveGroup: monitorTypeGroup
            text: i18n("Bar")
            onCheckedChanged: if (checked) {cfg_monitorType = 0; cpuAllActivatedCheckBox.checked = false;}
        }

        RadioButton {
            id: circularMonitorRadio
            exclusiveGroup: monitorTypeGroup
            text: i18n("Circular")
            onCheckedChanged: if (checked) {cfg_monitorType = 1; cpuAllActivatedCheckBox.checked = false;}
        }

        RadioButton {
            id: compactBarMonitorRadio
            exclusiveGroup: monitorTypeGroup
            text: i18n("Compact Bar")
            onCheckedChanged: if (checked) cfg_monitorType = 2;
        }

        Label {
            text: i18n("Update interval:")
            Layout.alignment: Qt.AlignRight
        }

        SpinBox {
            id: updateIntervalSpinBox
            decimals: 1
            stepSize: 0.1
            minimumValue: 0.1
            suffix: i18nc("Abbreviation for seconds", "s")
        }
    }

    Component.onCompleted: cfg_monitorTypeChanged()
}
