/*
 * SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "General")
         icon: "preferences-desktop-plasma"
         source: "GeneralSettings.qml"
    }
    ConfigCategory {
         name: i18nc("@title", "Colors")
         icon: "preferences-desktop-color"
         source: "ColorSettings.qml"
    }
}
