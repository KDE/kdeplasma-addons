/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
        name: i18nc("@title", "Weather Station")
        icon: "services"
        source: "config/ConfigWeatherStation.qml"
    }

    ConfigCategory {
        name: i18nc("@title", "Appearance")
        icon: "preferences-desktop-color"
        source: "config/ConfigAppearance.qml"
    }

    ConfigCategory {
        name: i18nc("@title", "Units")
        icon: "preferences-other"
        source: "config/ConfigUnits.qml"
    }
}
