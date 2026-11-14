/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

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

    LocationsControl {
        id: locations
    }

    ColumnLayout {
        spacing: Kirigami.Units.gridUnit

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: ['wettercom', 'dwd'].includes(Plasmoid.configuration.provider)
            text: i18nc("@label:info. %1 is the name of a weather service", "“%1” may not offer the current temperature or other weather conditions for this location.", locations.getProviderDisplayName(Plasmoid.configuration.provider))
        }

        Kirigami.Form {
            Layout.fillWidth: true
            Kirigami.FormGroup {
                // TODO: enable title once there are also settings for non-compact mode
                // title: i18nc("@title:group", "Compact Mode")
                visible: root.isInPanel
                QQC2.ButtonGroup { id: temperatureButtonGroup }
                Kirigami.FormEntry {
                    title: i18nc("@label", "Show temperature:")
                    contentItem: QQC2.RadioButton {
                        id: radioTempInBadge
                        QQC2.ButtonGroup.group: temperatureButtonGroup
                        checked: root.cfg_showTemperatureInCompactMode && (root.cfg_showTemperatureInBadge || root.needsToBeSquare)
                        onToggled: root.setShowTemperature(true, true)
                        text: i18nc("@option:radio Show temperature:", "Over the widget icon")
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.RadioButton {
                        id: radioTempBesideIcon
                        visible: !root.needsToBeSquare
                        QQC2.ButtonGroup.group: temperatureButtonGroup
                        checked: root.cfg_showTemperatureInCompactMode && !root.cfg_showTemperatureInBadge && !root.needsToBeSquare
                        onToggled: root.setShowTemperature(true, false)
                        text: i18nc("@option:radio Show temperature:", "Beside the widget icon")
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.RadioButton {
                        id: radioTempHide
                        QQC2.ButtonGroup.group: temperatureButtonGroup
                        checked: !root.cfg_showTemperatureInCompactMode && root.isInPanel
                        onToggled: root.setShowTemperature(false, false)
                        text: i18nc("@option:radio Show temperature:", "Do not show")
                    }
                }
            }

            Kirigami.FormGroup {
                visible: root.isInPanel
                Kirigami.FormEntry {
                    title: i18nc("@label", "Show in tooltip:")
                    contentItem: QQC2.CheckBox {
                        id: showTemperatureInTooltipCheckBox
                        text: i18nc("@option:check", "Temperature")
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.CheckBox {
                        id: showWindInTooltipCheckBox
                        text: i18nc("@option:check Show in tooltip: wind", "Wind")
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.CheckBox {
                        id: showPressureInTooltipCheckBox
                        text: i18nc("@option:check Show in tooltip: pressure", "Pressure")
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.CheckBox {
                        id: showHumidityInTooltipCheckBox
                        text: i18nc("@option:check Show in tooltip: humidity", "Humidity")
                    }
                }
            }

            Kirigami.FormGroup {
                Kirigami.FormEntry {
                    title: i18nc("@label", "Show temperature graph:")
                    contentItem: QQC2.CheckBox {
                        id: showHourlyTemperatureGraph
                        text: i18nc("@option:check", "Hourly Forecast")
                    }
                }

                Kirigami.FormEntry {
                    contentItem: QQC2.CheckBox {
                        id: showDayTemperatureGraph
                        text: i18nc("@option:check", "Day Forecast")
                    }
                }
            }
        }
    }
}
