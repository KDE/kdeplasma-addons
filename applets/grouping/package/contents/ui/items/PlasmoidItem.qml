/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.core 2.1 as PlasmaCore
// CHECK: do we need it here?
import org.kde.plasma.plasmoid 2.0

AbstractItem {
    id: plasmoidContainer

    property Item applet
    text: applet ? applet.plasmoid.title : ""

    itemId: applet ? applet.plasmoid.id : ""
    status: applet ? applet.plasmoid.status : PlasmaCore.Types.UnknownStatus
    active: root.activeApplet !== applet

    onClicked: {
        if (applet) {
            if (mouse.button === Qt.RightButton) {
                plasmoid.showPlasmoidMenu(applet, mouse.x, mouse.y);
            }
        }
    }
}
