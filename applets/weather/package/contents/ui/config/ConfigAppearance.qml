/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QtControls

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.private.weather 1.0


Kirigami.FormLayout {
    id: displayConfigPage

    readonly property bool canShowMoreInCompactMode: !plasmoid.nativeInterface.needsToBeSquare

    property alias cfg_showTemperatureInCompactMode: showTemperatureInCompactModeCheckBox.checked
    property alias cfg_showTemperatureInTooltip: showTemperatureInTooltipCheckBox.checked
    property alias cfg_showWindInTooltip: showWindInTooltipCheckBox.checked
    property alias cfg_showPressureInTooltip: showPressureInTooltipCheckBox.checked
    property alias cfg_showHumidityInTooltip: showHumidityInTooltipCheckBox.checked

    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Compact Mode")

        // TODO: conditionalize this once there are also settings for non-compact mode
        visible: false
    }

    QtControls.CheckBox {
        id: showTemperatureInCompactModeCheckBox
        Kirigami.FormData.label: i18nc("@label", "Show beside widget icon:")
        visible: canShowMoreInCompactMode
        text: i18nc("@option:check Show on widget icon: temperature", "Temperature")
    }

    Item {
        Kirigami.FormData.isSection: true
        visible: showTemperatureInCompactModeCheckBox.visible
    }

    QtControls.CheckBox {
        id: showTemperatureInTooltipCheckBox
        Kirigami.FormData.label: i18nc("@label", "Show in tooltip:")
        text: i18nc("@option:check", "Temperature")
    }

    QtControls.CheckBox {
        id: showWindInTooltipCheckBox
        text: i18nc("@option:check Show in tooltip: wind", "Wind")
    }

    QtControls.CheckBox {
        id: showPressureInTooltipCheckBox
        text: i18nc("@option:check Show in tooltip: pressure", "Pressure")
    }

    QtControls.CheckBox {
        id: showHumidityInTooltipCheckBox
        text: i18nc("@option:check Show in tooltip: humidity", "Humidity")
    }
}
