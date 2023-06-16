/*
 *  SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigami 2.20 as Kirigami

ListDelegate {
    id: item

    property bool interactiveIcon: false
    property alias source: avatar.source

    signal iconClicked()

    iconItem: Kirigami.Avatar {
        id: avatar

        anchors.fill: parent

        enabled: item.interactiveIcon // don't block mouse hover from the underlying ListView highlight
        actions.main: Kirigami.Action {
            onTriggered: item.iconClicked()
        }

        iconSource: "user-identity"

        border.color: Kirigami.ColorUtils.adjustColor(Kirigami.Theme.textColor, {alpha: 0.4*255})
        border.width: Kirigami.Units.devicePixelRatio
    }
}
