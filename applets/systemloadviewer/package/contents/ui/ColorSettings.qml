/*
 * Copyright (C) 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 * Copyright (C) 2019 Nate Graham <nate@kde.org>
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

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls

Kirigami.FormLayout {
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
    property alias cfg_setColorsManually: setColorsManually.checked

    QQC2.CheckBox {
        id: setColorsManually
        text: i18nc("@option:check", "Set Colors Manually")
    }

    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "CPU")
        enabled: setColorsManually.checked
    }


    KQuickControls.ColorButton {
        id: cpuUserColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "User:")
        enabled: setColorsManually.checked
    }

    KQuickControls.ColorButton {
        id: cpuIOWaitColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "IOWait:")
        enabled: setColorsManually.checked
    }

    KQuickControls.ColorButton {
        id: cpuSysColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "Sys:")
        enabled: setColorsManually.checked
    }

    KQuickControls.ColorButton {
        id: cpuNiceColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "Nice:")
        enabled: setColorsManually.checked
    }


    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Memory")
        enabled: setColorsManually.checked
    }


    KQuickControls.ColorButton {
        id: memApplicationColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "Application:")
        enabled: setColorsManually.checked
    }

    KQuickControls.ColorButton {
        id: memBuffersColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "Buffers:")
        enabled: setColorsManually.checked
    }


    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Swap")
        enabled: setColorsManually.checked
    }


    KQuickControls.ColorButton {
        id: swapUsedColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "Used swap:")
        enabled: setColorsManually.checked
    }


    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Cache")
        enabled: setColorsManually.checked
    }


    KQuickControls.ColorButton {
        id: cacheDirtyColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "Dirty memory:")
        enabled: setColorsManually.checked
    }

    KQuickControls.ColorButton {
        id: cacheWritebackColorPicker
        Kirigami.FormData.label: i18nc("@label:chooser", "Writeback memory:")
        enabled: setColorsManually.checked
    }
}
