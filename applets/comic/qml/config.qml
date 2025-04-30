/*
 *  SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "Comics")
         icon: "face-smile-big"
         source: "configComics.qml"
    }
    ConfigCategory {
         name: i18nc("@title", "Behavior")
         icon: "preferences-desktop"
         source: "configBehavior.qml"
    }
}
