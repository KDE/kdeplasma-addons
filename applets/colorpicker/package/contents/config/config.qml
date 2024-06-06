/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
        name: i18nc("@title", "General")
        icon: "preferences-desktop-plasma"
        source: "configGeneral.qml"
    }
}
