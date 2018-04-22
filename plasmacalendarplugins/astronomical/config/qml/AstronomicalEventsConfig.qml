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

import QtQuick 2.0

import QtQuick.Controls 1.2 as QtControls
import QtQuick.Layouts 1.0

import org.kde.plasmacalendar.astronomicaleventsconfig 1.0

ColumnLayout {
    id: configPage

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

    QtControls.CheckBox {
        id: showLunarPhasesCheckBox

        checked: configStorage.isLunarPhaseShown
        text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Show lunar phases")
        onCheckedChanged: configPage.configurationChanged();
    }

    QtControls.CheckBox {
        id: showSeasonsCheckBox

        checked: configStorage.isSeasonShown
        text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Show astronomical seasons (solstices and equinoxes)")
        onCheckedChanged: configPage.configurationChanged();
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
