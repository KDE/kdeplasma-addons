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
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: generalSettings

    property alias cfg_cpuActivated: cpuActivatedCheckBox.checked
    property alias cfg_cpuAllActivated: cpuAllActivatedCheckBox.checked
    property alias cfg_memoryActivated: memoryActivatedCheckBox.checked
    property alias cfg_swapActivated: swapActivatedCheckBox.checked
    property alias cfg_cacheActivated: cacheActivatedCheckBox.checked
    property alias cfg_updateInterval: updateIntervalSpinBox.value
    property int cfg_monitorType: plasmoid.configuration.monitorType

    QQC2.ButtonGroup {
        id: monitorTypeGroup
    }

    QQC2.CheckBox {
        id: cpuActivatedCheckBox

        Kirigami.FormData.label: i18nc("@label", "Show:")

        text: i18nc("@option:check", "CPU monitor")
        onCheckedChanged: if (!checked) {cpuAllActivatedCheckBox.checked = false;}
    }

    RowLayout {
        Layout.fillWidth: true

        Item {
            width: Kirigami.Units.gridUnit
        }
        QQC2.CheckBox {
            id: cpuAllActivatedCheckBox

            Layout.fillWidth: true

            text: i18nc("@option:check", "CPUs separately")
            enabled: cpuActivatedCheckBox.checked && cfg_monitorType === 2
        }
    }

    QQC2.CheckBox {
        id: memoryActivatedCheckBox
        text: i18nc("@option:check", "Memory monitor")
    }

    QQC2.CheckBox {
        id: swapActivatedCheckBox
        text: i18nc("@option:check", "Swap monitor")
    }

    QQC2.CheckBox {
        id: cacheActivatedCheckBox
        text: i18nc("@option:check", "Cache monitor")
    }


    Item {
        Kirigami.FormData.isSection: true
    }


    QQC2.RadioButton {
        id: barMonitorRadio
        QQC2.ButtonGroup.group: monitorTypeGroup

        Kirigami.FormData.label: i18nc("@label", "Monitor type:")

        text: i18nc("@option:radio", "Bar")

        checked: cfg_monitorType == 0
        onClicked: if (checked) {cfg_monitorType = 0; cpuAllActivatedCheckBox.checked = false;}
    }

    QQC2.RadioButton {
        id: circularMonitorRadio
        QQC2.ButtonGroup.group: monitorTypeGroup

        text: i18nc("@option:radio", "Circular")

        checked: cfg_monitorType == 1
        onClicked: if (checked) {cfg_monitorType = 1; cpuAllActivatedCheckBox.checked = false;}
    }

    QQC2.RadioButton {
        id: compactBarMonitorRadio
        QQC2.ButtonGroup.group: monitorTypeGroup

        text: i18nc("@option:radio", "Compact bar")

        checked: cfg_monitorType == 2
        onClicked: if (checked) cfg_monitorType = 2;
    }


        Item {
        Kirigami.FormData.isSection: true
    }

    // QQC2 SpinBox doesn't cleanly support non-integer values, which can be worked
    // around, but the code is messy and the user experience is somewhat poor.
    // So for now, we stick with the QQC1 SpinBox
    QQC1.SpinBox {
        id: updateIntervalSpinBox

        Kirigami.FormData.label: i18nc("@label:spinbox", "Update interval:")

        decimals: 1
        stepSize: 0.1
        minimumValue: 0.1
        suffix: i18ncp("@item:valuesuffix spacing to number + unit (seconds)", " second", " seconds")
    }
}
