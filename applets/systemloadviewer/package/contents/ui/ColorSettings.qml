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
    id: colorSettings

    property alias cfg_memApplicationColor: memApplicationColorPicker.chosenColor
    property alias cfg_memBuffersColor: memBuffersColorPicker.chosenColor
    property alias cfg_memCachedColor: memCachedColorPicker.chosenColor
    property alias cfg_cpuUserColor: cpuUserColorPicker.chosenColor
    property alias cfg_cpuIOWaitColor: cpuIOWaitColorPicker.chosenColor
    property alias cfg_cpuSysColor: cpuSysColorPicker.chosenColor
    property alias cfg_cpuNiceColor: cpuNiceColorPicker.chosenColor
    property alias cfg_swapUsedColor: swapUsedColorPicker.chosenColor
    property alias cfg_cacheDirtyColor: cacheDirtyColorPicker.chosenColor
    property alias cfg_cacheWritebackColor: cacheWritebackColorPicker.chosenColor
    property alias cfg_setColorsManually: setColorsManuallyGroupBox.checked

    GroupBox {
        id: setColorsManuallyGroupBox
        title: i18n("Set colors manually")
        checkable: true
        flat: true

        width: parent.width
        height: parent.height

        GridLayout {
            columns: 2
            anchors.left: parent.left
            anchors.leftMargin: units.largeSpacing

            Label {
                text: i18n("CPU")
                Layout.columnSpan: 2
                font.bold: true
            }

            Label {
                text: i18n("User:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: cpuUserColorPicker
            }

            Label {
                text: i18n("IOWait:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: cpuIOWaitColorPicker
            }

            Label {
                text: i18n("Sys:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: cpuSysColorPicker
            }

            Label {
                text: i18n("Nice:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: cpuNiceColorPicker
            }

            Label {
                text: i18n("Memory")
                Layout.columnSpan: 2
                font.bold: true
            }


            Label {
                text: i18n("Application:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: memApplicationColorPicker
            }

            Label {
                text: i18n("Buffers:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: memBuffersColorPicker
            }

            Label {
                text: i18n("Cached:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: memCachedColorPicker
            }

            Label {
                text: i18n("Swap")
                Layout.columnSpan: 2
                font.bold: true
            }


            Label {
                text: i18n("Used swap:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: swapUsedColorPicker
            }

            Label {
                text: i18n("Cache")
                Layout.columnSpan: 2
                font.bold: true
            }

            Label {
                text: i18n("Dirty memory:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: cacheDirtyColorPicker
            }

            Label {
                text: i18n("Writeback memory:")
                Layout.alignment: Qt.AlignRight
            }

            ColorPicker {
                id: cacheWritebackColorPicker
            }
        }
    }
}
