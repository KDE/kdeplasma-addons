/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasmacalendar.alternatecalendarconfig 1.0

KCM.SimpleKCM {
    id: configPage

    // expected API
    signal configurationChanged

    // expected API
    function saveConfig() {
        configStorage.calendarSystem = calendarSystemComboBox.currentValue;
        configStorage.dateOffset = dateOffsetSpinBoxLoader.active && dateOffsetSpinBoxLoader.item.value || 0;

        configStorage.save();
    }

    Kirigami.FormLayout {

        ConfigStorage {
            id: configStorage
        }

        Row {
            Kirigami.FormData.label: i18ndc("plasma_calendar_alternatecalendar", "@label:listbox", "Calendar system:")

            QQC2.ComboBox {
                id: calendarSystemComboBox
                model: configStorage.calendarSystemModel
                textRole: "display"
                valueRole: "id"
                currentIndex: configStorage.currentIndex
                onActivated: configPage.configurationChanged();
            }

            KCM.ContextualHelpButton {
                anchors.verticalCenter: calendarSystemComboBox.verticalCenter
                visible: calendarSystemComboBox.currentValue === "Islamic"
                toolTipText: i18ndc("plasma_calendar_alternatecalendar", "@info:tooltip", "This calendar is based on pure astronomical calculation. It doesn't consider any crescent visibility criteria.")
            }
        }

        Loader {
            id: dateOffsetSpinBoxLoader
            active: calendarSystemComboBox.currentValue.startsWith("Islamic")
            visible: active
            Kirigami.FormData.label: i18ndc("plasma_calendar_alternatecalendar", "@label:spinbox", "Date offset:")

            sourceComponent: QQC2.SpinBox {
                hoverEnabled: true

                stepSize: 1
                from: -10
                to: 10
                value: configStorage.dateOffset
                onValueChanged: configPage.configurationChanged()

                textFromValue: (value, locale) => i18ndp("plasma_calendar_alternatecalendar","%1 day", "%1 days", value)
                valueFromText: (text, locale) => parseInt(text)

                QQC2.ToolTip.text: i18ndc("plasma_calendar_alternatecalendar", "@info:tooltip", "A positive offset signifies a later date, while a negative offset signifies an earlier date.")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        }
    }
}
