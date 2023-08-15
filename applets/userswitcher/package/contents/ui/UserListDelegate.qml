/*
 *  SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15

import org.kde.kcmutils // For KCMLauncher
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.components 1.0 as KirigamiComponents

ListDelegate {
    id: item

    property alias source: avatar.source

    iconItem: KirigamiComponents.AvatarButton {
        id: avatar

        anchors.fill: parent

        name: item.text

        // don't block mouse hover from the underlying ListView highlight
        enabled: KConfig.KAuthorized.authorizeControlModule("kcm_users")

        onClicked: KCMLauncher.openSystemSettings("kcm_users")
    }
}
