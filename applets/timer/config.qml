/*
 *  SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "General")
         icon: "kalarm"
         source: "configGeneral.qml"
    }

    ConfigCategory {
         name: i18nc("@title", "Predefined Timers")
         icon: "chronometer"
         source: "configTimes.qml"
    }
}
