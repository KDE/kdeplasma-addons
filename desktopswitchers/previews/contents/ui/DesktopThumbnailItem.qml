/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.12
import org.kde.kwin 3.0 as KWinComponents

Item {
    id: root

    property int desktop: 1

    Item {
        id: container
        anchors.centerIn: parent
        width: KWinComponents.Workspace.virtualScreenSize.width
        height: KWinComponents.Workspace.virtualScreenSize.height
        scale: Math.min(parent.width / container.width, parent.height / container.height)

        Repeater {
            model: KWinComponents.ClientFilterModel {
                activity: KWinComponents.Workspace.currentActivity
                model: KWinComponents.ClientModel {}
            }

            KWinComponents.WindowThumbnailItem {
                client: model.client
                x: model.client.x
                y: model.client.y
                z: model.client.stackingOrder
                visible: (model.client.desktop === -1 || model.client.desktop === root.desktop) && !model.client.minimized
            }
        }
    }
}
