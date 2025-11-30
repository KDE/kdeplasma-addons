/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "Media")
         icon: "folder-image"
         source: "ConfigMedia.qml"
    }
    ConfigCategory {
         name: i18nc("@title", "Behavior")
         icon: "image"
         source: "ConfigBehavior.qml"
    }
}
