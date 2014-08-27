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

    property alias cfg_memApplicationColor: memApplicationColorPicker.chosenColor
    property alias cfg_memBuffersColor: memBuffersColorPicker.chosenColor
    property alias cfg_memCachedColor: memCachedColorPicker.chosenColor
    property alias cfg_cpuUserColor: cpuUserColorPicker.chosenColor
    property alias cfg_cpuIOWaitColor: cpuIOWaitColorPicker.chosenColor
    property alias cfg_cpuSysColor: cpuSysColorPicker.chosenColor
    property alias cfg_cpuNiceColor: cpuNiceColorPicker.chosenColor
    property alias cfg_swapUsedColor: swapUsedColorPicker.chosenColor
    property alias cfg_cpuActivated: cpuActivatedCheckBox.checked
    property alias cfg_memoryActivated: memoryActivatedCheckBox.checked
    property alias cfg_swapActivated: swapActivatedCheckBox.checked
    property alias cfg_useThemeColors: useThemeColorsCheckBox.checked
    property alias cfg_updateInterval: updateIntervalSpinBox.value
    property int cfg_monitorType

    height: layout.implicitHeight
    width: layout.implicitWidth

    onCfg_monitorTypeChanged: {
        switch (cfg_monitorType) {
            default: case 0: monitorTypeGroup.current = barMonitorRadio; break;
            case 1: monitorTypeGroup.current = circularMonitorRadio; break;
        }
    }

    ExclusiveGroup {
        id: monitorTypeGroup
    }

    GridLayout {
        id: layout

        columns: 2

        Label {
            text: i18n("Update interval (seconds):")
            Layout.alignment: Qt.AlignRight
        }

        SpinBox {
            id: updateIntervalSpinBox
            decimals: 1
            stepSize: 0.1
            minimumValue: 0.1
        }

        Label {
            text: i18n("Use theme colors:")
            Layout.alignment: Qt.AlignRight
        }

        CheckBox {
            id: useThemeColorsCheckBox
            text: i18n("Enabled")
        }

        Label {
            text: i18n("Monitor type:")
            Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }

        RadioButton {
            id: barMonitorRadio
            exclusiveGroup: monitorTypeGroup
            text: i18n("Bar")
            onCheckedChanged: if (checked) cfg_monitorType = 0;
        }

        Item {
            width: 2
            height: 2
            Layout.rowSpan: 1
        }

        RadioButton {
            id: circularMonitorRadio
            exclusiveGroup: monitorTypeGroup
            text: i18n("Circular")
            onCheckedChanged: if (checked) cfg_monitorType = 1;
        }

        CheckBox {
            id: cpuActivatedCheckBox
            text: i18n("CPU monitor")
            Layout.columnSpan: 2
        }

        Label {
            text: i18n("User:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: cpuUserColorPicker
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        Label {
            text: i18n("IOWait:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: cpuIOWaitColorPicker
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        Label {
            text: i18n("Sys:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: cpuSysColorPicker
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        Label {
            text: i18n("Nice:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: cpuNiceColorPicker
            visible: cfg_cpuActivated && !cfg_useThemeColors
        }

        CheckBox {
            id: memoryActivatedCheckBox
            text: i18n("Memory monitor")
            Layout.columnSpan: 2
        }

        Label {
            text: i18n("Application:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_memoryActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: memApplicationColorPicker
            visible: cfg_memoryActivated && !cfg_useThemeColors
        }

        Label {
            text: i18n("Buffers:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_memoryActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: memBuffersColorPicker
            visible: cfg_memoryActivated && !cfg_useThemeColors
        }

        Label {
            text: i18n("Cached:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_memoryActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: memCachedColorPicker
            visible: cfg_memoryActivated && !cfg_useThemeColors
        }

        CheckBox {
            id: swapActivatedCheckBox
            text: i18n("Swap monitor")
            Layout.columnSpan: 2
        }

        Label {
            text: i18n("Used swap:")
            Layout.alignment: Qt.AlignRight
            visible: cfg_swapActivated && !cfg_useThemeColors
        }

        ColorPicker {
            id: swapUsedColorPicker
            visible: cfg_swapActivated && !cfg_useThemeColors
        }
    }

    Component.onCompleted: cfg_monitorTypeChanged()
}
