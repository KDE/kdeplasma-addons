/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QtControls

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.weather 1.0


Kirigami.FormLayout {
    id: displayConfigPage

    readonly property bool canShowMoreInCompactMode: !Plasmoid.nativeInterface.needsToBeSquare

    property bool cfg_showTemperatureInCompactMode
    property bool cfg_showTemperatureInBadge

    property alias cfg_showTemperatureInTooltip: showTemperatureInTooltipCheckBox.checked
    property alias cfg_showWindInTooltip: showWindInTooltipCheckBox.checked
    property alias cfg_showPressureInTooltip: showPressureInTooltipCheckBox.checked
    property alias cfg_showHumidityInTooltip: showHumidityInTooltipCheckBox.checked

    function setShowTemperature(inCompactMode, inBadge) {
        cfg_showTemperatureInCompactMode = inCompactMode
        cfg_showTemperatureInBadge = inBadge
    }

    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Compact Mode")

        // TODO: conditionalize this once there are also settings for non-compact mode
        visible: false
    }

    QtControls.RadioButton {
        id: radioTempInBadge
        Kirigami.FormData.label: i18nc("@label", "Show temperature:")
        checked: cfg_showTemperatureInCompactMode && (cfg_showTemperatureInBadge || !canShowMoreInCompactMode)
        onToggled: setShowTemperature(true, true)
        text: i18nc("@option:radio Show temperature:", "Over the widget icon")
    }

    QtControls.RadioButton {
        id: radioTempBesideIcon
        visible: canShowMoreInCompactMode
        checked: cfg_showTemperatureInCompactMode && !cfg_showTemperatureInBadge && canShowMoreInCompactMode
        onToggled: setShowTemperature(true, false)
        text: i18nc("@option:radio Show temperature:", "Beside the widget icon")
    }

    QtControls.RadioButton {
        id: radioTempHide
        checked: !cfg_showTemperatureInCompactMode
        onToggled: setShowTemperature(false, false)
        text: i18nc("@option:radio Show temperature:", "Do not show")
    }

    Item {
        Kirigami.FormData.isSection: true
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
