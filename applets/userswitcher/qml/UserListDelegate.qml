/*
 *  SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQml

import org.kde.config as KConfig
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.plasma.components as PC3

ListDelegate {
    id: item

    property alias source: avatar.source

    iconItem: KirigamiComponents.AvatarButton {
        id: avatar

        anchors.fill: parent

        name: item.text
        PC3.ToolTip.text: Accessible.description
        Accessible.description: i18nc("@action:button @info:tooltip accessible","Open user configuration")

        // don't block mouse hover from the underlying ListView highlight
        enabled: KConfig.KAuthorized.authorizeControlModule("kcm_users")

        onClicked: KCMUtils.KCMLauncher.openSystemSettings("kcm_users")
    }
}
