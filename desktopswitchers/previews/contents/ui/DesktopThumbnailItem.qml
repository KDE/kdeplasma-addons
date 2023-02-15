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
            model: KWinComponents.WindowFilterModel {
                activity: KWinComponents.Workspace.currentActivity
                model: KWinComponents.WindowModel {}
            }

            KWinComponents.WindowThumbnailItem {
                client: model.window
                x: model.window.x
                y: model.window.y
                z: model.window.stackingOrder
                visible: (model.window.desktop === -1 || model.window.desktop === root.desktop) && !model.window.minimized
            }
        }
    }
}
