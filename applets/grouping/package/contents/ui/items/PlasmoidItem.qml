/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.core 2.1 as PlasmaCore

AbstractItem {
    id: plasmoidContainer

    property Item applet
    text: applet ? applet.title : ""

    itemId: applet ? applet.id : ""
    status: applet ? applet.status : PlasmaCore.Types.UnknownStatus
    active: root.activeApplet !== applet

    onClicked: {
        if (applet) {
            if (mouse.button === Qt.RightButton) {
                plasmoid.nativeInterface.showPlasmoidMenu(applet, mouse.x, mouse.y);
            }
        }
    }
}
