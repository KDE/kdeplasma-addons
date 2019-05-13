/*
    Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.plasmacalendar.astronomicaleventsconfig 1.0
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: configPage

    anchors.left: parent.left
    anchors.right: parent.right

    // expected API
    signal configurationChanged

    // expected API
    function saveConfig()
    {
        configStorage.isLunarPhaseShown = showLunarPhasesCheckBox.checked;
        configStorage.isSeasonShown = showSeasonsCheckBox.checked;

        configStorage.save();
    }

    ConfigStorage {
        id: configStorage
    }

    QQC2.CheckBox {
        id: showLunarPhasesCheckBox

        Kirigami.FormData.label: i18n("Show:")

        checked: configStorage.isLunarPhaseShown
        text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Lunar phases")
        onCheckedChanged: configPage.configurationChanged();
    }

    QQC2.CheckBox {
        id: showSeasonsCheckBox

        checked: configStorage.isSeasonShown
        text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Astronomical seasons (solstices and equinoxes)")
        onCheckedChanged: configPage.configurationChanged();
    }
}
