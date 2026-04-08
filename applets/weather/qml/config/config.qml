/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
        name: i18nc("@title", "Weather Station")
        icon: "services"
        source: "ConfigWeatherStation.qml"
    }

    ConfigCategory {
        name: i18nc("@title", "Appearance")
        icon: "preferences-desktop-color"
        source: "ConfigAppearance.qml"
    }

    ConfigCategory {
        name: i18nc("@title", "Units")
        icon: "preferences-other"
        source: "ConfigUnits.qml"
    }
}
