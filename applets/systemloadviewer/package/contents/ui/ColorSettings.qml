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
import org.kde.kquickcontrols 2.0 as KQuickControls

ColumnLayout {
    id: colorSettings

    property alias cfg_memApplicationColor: memApplicationColorPicker.color
    property alias cfg_memBuffersColor: memBuffersColorPicker.color
    property alias cfg_cpuUserColor: cpuUserColorPicker.color
    property alias cfg_cpuIOWaitColor: cpuIOWaitColorPicker.color
    property alias cfg_cpuSysColor: cpuSysColorPicker.color
    property alias cfg_cpuNiceColor: cpuNiceColorPicker.color
    property alias cfg_swapUsedColor: swapUsedColorPicker.color
    property alias cfg_cacheDirtyColor: cacheDirtyColorPicker.color
    property alias cfg_cacheWritebackColor: cacheWritebackColorPicker.color
    property alias cfg_setColorsManually: setColorsManuallyGroupBox.checked

    GroupBox {
        id: setColorsManuallyGroupBox
        Layout.fillWidth: true
        flat: true

        title: i18nc("@option:check", "Set Colors Manually")
        checkable: true

        GridLayout {
            columns: 2

            Label {
                text: i18nc("@title:group", "CPU")
                Layout.columnSpan: 2
                font.bold: true
            }

            Label {
                text: i18nc("@label:chooser", "User:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: cpuUserColorPicker
            }

            Label {
                text: i18nc("@label:chooser", "IOWait:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: cpuIOWaitColorPicker
            }

            Label {
                text: i18nc("@label:chooser", "Sys:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: cpuSysColorPicker
            }

            Label {
                text: i18nc("@label:chooser", "Nice:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: cpuNiceColorPicker
            }

            Label {
                text: i18nc("@title:group", "Memory")
                Layout.columnSpan: 2
                font.bold: true
            }


            Label {
                text: i18nc("@label:chooser", "Application:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: memApplicationColorPicker
            }

            Label {
                text: i18nc("@label:chooser", "Buffers:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: memBuffersColorPicker
            }

            Label {
                text: i18nc("@title:group", "Swap")
                Layout.columnSpan: 2
                font.bold: true
            }


            Label {
                text: i18nc("@label:chooser", "Used swap:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: swapUsedColorPicker
            }

            Label {
                text: i18nc("@title:group", "Cache")
                Layout.columnSpan: 2
                font.bold: true
            }

            Label {
                text: i18nc("@label:chooser", "Dirty memory:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: cacheDirtyColorPicker
            }

            Label {
                text: i18nc("@label:chooser", "Writeback memory:")
                Layout.alignment: Qt.AlignRight
            }

            KQuickControls.ColorButton {
                id: cacheWritebackColorPicker
            }
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
