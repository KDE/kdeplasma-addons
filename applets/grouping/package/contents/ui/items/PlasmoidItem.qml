/*
 *   Copyright 2015 Marco Martin <mart@kde.org>
 *  Copyright 2016 David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

AbstractItem {
    id: plasmoidContainer

    property Item applet
    text: applet ? applet.title : ""

    itemId: applet ? applet.id : ""
    status: applet ? applet.status : PlasmaCore.Types.UnknownStatus
    active: root.activeApplet != applet

    onClicked: {
        if (applet) {
            if (mouse.button == Qt.RightButton) {
                plasmoid.nativeInterface.showPlasmoidMenu(applet, mouse.x, mouse.y);
            }
        }
    }
}
