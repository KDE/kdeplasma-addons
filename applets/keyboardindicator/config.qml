/*
 *  SPDX-FileCopyrightText: 2017 Bernhard Friedreich <friesoft@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "Keys")
         icon: "input-caps-on"
         source: "configAppearance.qml"
    }
}
