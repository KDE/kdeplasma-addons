/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.plasmacalendar.astronomicaleventsconfig
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: configPage

    // expected API
    signal configurationChanged

    property bool unsavedChanges: false

    // expected API
    function saveConfig()
    {
        configStorage.isLunarPhaseShown = showLunarPhasesCheckBox.checked;
        configStorage.isSeasonShown = showSeasonsCheckBox.checked;

        configStorage.save();
        unsavedChanges = false;
    }

    function checkUnsavedChanges() {
        unsavedChanges = !(configStorage.isLunarPhaseShown === showLunarPhasesCheckBox.checked &&
                           configStorage.isSeasonShown === showSeasonsCheckBox.checked)
    }

    Kirigami.FormLayout {

        ConfigStorage {
            id: configStorage
        }

        QQC2.CheckBox {
            id: showLunarPhasesCheckBox

            Kirigami.FormData.label: i18nd("plasma_calendar_astronomicalevents", "Show:")

            checked: configStorage.isLunarPhaseShown
            text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Lunar phases")
            onCheckedChanged: configPage.checkUnsavedChanges();
        }

        QQC2.CheckBox {
            id: showSeasonsCheckBox

            checked: configStorage.isSeasonShown
            text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Astronomical seasons (solstices and equinoxes)")
            onCheckedChanged: configPage.checkUnsavedChanges();
        }
    }
}
