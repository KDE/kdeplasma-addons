/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property alias cfg_randomize: randomizeCheckBox.checked
    property alias cfg_pauseOnMouseOver: pauseOnMouseOverCheckBox.checked
    property alias cfg_leftClickOpenImage: leftClickOpenImageCheckBox.checked

    property int cfg_fillMode: Image.PreserveAspectFit
    property int cfg_interval: 0
    property int hoursIntervalValue: Math.floor(cfg_interval / 3600)
    property int minutesIntervalValue: Math.floor(cfg_interval % 3600) / 60
    property int secondsIntervalValue: cfg_interval % 3600 % 60

    Kirigami.FormLayout {
        TextMetrics {
            id: textMetrics
            text: "00"
        }

        //FIXME: there should be only one spinbox: QtControls spinboxes are still too limited for it tough
        RowLayout {
            Layout.fillWidth: true

            Kirigami.FormData.label: i18n("Change picture every:")

            Connections {
                target: root
                function onHoursIntervalValueChanged() {hoursInterval.value = root.hoursIntervalValue}
                function onMinutesIntervalValueChanged() {minutesInterval.value = root.minutesIntervalValue}
                function onSecondsIntervalValueChanged() {secondsInterval.value = root.secondsIntervalValue}
            }
            SpinBox {
                id: hoursInterval
                value: root.hoursIntervalValue
                from: 0
                to: 24
                editable: true
                onValueChanged: root.cfg_interval = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value
            }
            Label {
                text: i18n("Hours")
            }
            Item {
                Layout.preferredWidth: Kirigami.Units.gridUnit
            }
            SpinBox {
                id: minutesInterval
                value: root.minutesIntervalValue
                from: 0
                to: 60
                editable: true
                onValueChanged: root.cfg_interval = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value
            }
            Label {
                text: i18n("Minutes")
            }
            Item {
                Layout.preferredWidth: Kirigami.Units.gridUnit
            }
            SpinBox {
                id: secondsInterval
                value: root.secondsIntervalValue
                from: root.hoursIntervalValue === 0 && root.minutesIntervalValue === 0 ? 1 : 0
                to: 60
                editable: true
                onValueChanged: root.cfg_interval = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value
            }
            Label {
                text: i18n("Seconds")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        ComboBox {
            id: comboBox
            Kirigami.FormData.label: i18nc("@label:listbox", "Image fill mode:")

            currentIndex: model.findIndex(entry => entry.value == root.cfg_fillMode)
            textRole: "text"
            model: [
                {
                    text: i18nc("@item:inlistbox", "Stretch"),
                    description: i18nc("@info", "The image is scaled to fit the frame"),
                    value: Image.Stretch
                },
                {
                    text: i18nc("@item:inlistbox", "Preserve aspect fit"),
                    description: i18nc("@info", "The image is scaled uniformly to fit without cropping"),
                    value: Image.PreserveAspectFit
                },
                {
                    text: i18nc("@item:inlistbox", "Preserve aspect crop"),
                    description: i18nc("@info", "The image is scaled uniformly to fill, cropping if necessary"),
                    value: Image.PreserveAspectCrop
                },
                {
                    text: i18nc("@item:inlistbox", "Tile"),
                    description: i18nc("@info", "The image is duplicated horizontally and vertically"),
                    value: Image.Tile
                },
                {
                    text: i18nc("@item:inlistbox", "Tile vertically"),
                    description: i18nc("@info", "The image is stretched horizontally and tiled vertically"),
                    value: Image.TileVertically
                },
                {
                    text: i18nc("@item:inlistbox", "Tile horizontally"),
                    description: i18nc("@info", "The image is stretched vertically and tiled horizontally"),
                    value: Image.TileHorizontally
                },
                {
                    text: i18nc("@item:inlistbox", "Pad"),
                    description: i18nc("@info", "The image is not transformed"),
                    value: Image.Pad
                }
            ]

            onActivated: index => root.cfg_fillMode = comboBox.model[index].value
        }

        Label {
            id: fillModeDescription
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: comboBox.model[comboBox.currentIndex] ? comboBox.model[comboBox.currentIndex].description : ""
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        CheckBox {
            id: randomizeCheckBox
            Kirigami.FormData.label: i18nc("@label:checkbox", "General:")
            text: i18nc("@option:check", "Randomize order")
        }

        CheckBox {
            id: pauseOnMouseOverCheckBox
            text: i18nc("@option:check", "Pause slideshow when cursor is over image")
        }

        CheckBox {
            id: leftClickOpenImageCheckBox
            text: i18nc("@option:check", "Click on image to open in external application")
        }
    }
}
