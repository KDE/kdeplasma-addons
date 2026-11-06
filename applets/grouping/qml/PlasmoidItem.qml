/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

AbstractItem {
    id: plasmoidContainer

    property Item applet
    text: applet ? applet.Plasmoid.title : ""

    itemId: applet ? applet.Plasmoid.id : -1
    status: applet ? applet.Plasmoid.status : PlasmaCore.Types.UnknownStatus
    active: root.activeApplet !== applet

    Component.onCompleted: console.log(root, root.activeApplet)

    onClicked: event => {
        if (applet) {
            if (event.button === Qt.RightButton) {
                Plasmoid.showPlasmoidMenu(applet, event.x, event.y);
            }
        }
    }
}
