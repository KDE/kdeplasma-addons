/*
 * SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 * SPDX-FileCopyrightText: 2019 Nate Graham <nate@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
