/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root
    readonly property bool needsToBeSquare: (Plasmoid.containmentType & PlasmaCore.Types.CustomEmbeddedContainment) || (Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentForcesSquarePlasmoids)
    readonly property bool isInPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge, PlasmaCore.Types.RightEdge,].includes(Plasmoid.location)

    property bool cfg_showTemperatureInCompactMode
    property bool cfg_showTemperatureInBadge

    property alias cfg_showTemperatureInTooltip: showTemperatureInTooltipCheckBox.checked
    property alias cfg_showWindInTooltip: showWindInTooltipCheckBox.checked
    property alias cfg_showPressureInTooltip: showPressureInTooltipCheckBox.checked
    property alias cfg_showHumidityInTooltip: showHumidityInTooltipCheckBox.checked

    property alias cfg_showHourlyTemperatureGraph: showHourlyTemperatureGraph.checked
    property alias cfg_showDayTemperatureGraph: showDayTemperatureGraph.checked

    function setShowTemperature(inCompactMode, inBadge) {
        cfg_showTemperatureInCompactMode = inCompactMode;
        cfg_showTemperatureInBadge = inBadge;
    }

    Kirigami.FormLayout {
        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18nc("@title:group", "Compact Mode")

            // TODO: conditionalize this once there are also settings for non-compact mode
            visible: false
        }

        QQC2.RadioButton {
            id: radioTempInBadge
            visible: root.isInPanel
            Kirigami.FormData.label: i18nc("@label", "Show temperature:")
            checked: cfg_showTemperatureInCompactMode && (cfg_showTemperatureInBadge || needsToBeSquare)
            onToggled: setShowTemperature(true, true)
            text: i18nc("@option:radio Show temperature:", "Over the widget icon")
        }

        QQC2.RadioButton {
            id: radioTempBesideIcon
            visible: !needsToBeSquare && root.isInPanel
            checked: cfg_showTemperatureInCompactMode && !cfg_showTemperatureInBadge && !needsToBeSquare
            onToggled: setShowTemperature(true, false)
            text: i18nc("@option:radio Show temperature:", "Beside the widget icon")
        }

        QQC2.RadioButton {
            id: radioTempHide
            visible: root.isInPanel
            checked: !cfg_showTemperatureInCompactMode
            onToggled: setShowTemperature(false, false)
            text: i18nc("@option:radio Show temperature:", "Do not show")
        }

        Item {
            visible: root.isInPanel
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: showTemperatureInTooltipCheckBox
            visible: root.isInPanel
            Kirigami.FormData.label: i18nc("@label", "Show in tooltip:")
            text: i18nc("@option:check", "Temperature")
        }

        QQC2.CheckBox {
            id: showWindInTooltipCheckBox
            visible: root.isInPanel
            text: i18nc("@option:check Show in tooltip: wind", "Wind")
        }

        QQC2.CheckBox {
            id: showPressureInTooltipCheckBox
            visible: root.isInPanel
            text: i18nc("@option:check Show in tooltip: pressure", "Pressure")
        }

        QQC2.CheckBox {
            id: showHumidityInTooltipCheckBox
            visible: root.isInPanel
            text: i18nc("@option:check Show in tooltip: humidity", "Humidity")
        }

        Item {
            visible: root.isInPanel
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: showHourlyTemperatureGraph
            Kirigami.FormData.label: i18nc("@label", "Show temperature graph:")
            text: i18nc("@option:check", "Hourly Forecast")
        }

        QQC2.CheckBox {
            id: showDayTemperatureGraph
            text: i18nc("@option:check", "Day Forecast")
        }
    }
}
