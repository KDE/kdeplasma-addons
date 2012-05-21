/*
 *   Copyright 2012 Alex Merry <alex.merry@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: root
    property alias artUrl: realArt.source

    implicitWidth: realArt.visible ? realArt.implicitWidth : noArtItem.implicitWidth
    implicitHeight: realArt.visible ? realArt.implicitHeight : noArtItem.implicitHeight

    Image {
        id: realArt
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        visible: (status == Image.Ready)
    }
    PlasmaCore.Svg {
        id: noArtSvg
        imagePath: plasmoid.file("images", "nocover.svgz")
    }
    // TODO: figure out how to centre this vertically
    // TODO: use the media player logo, if available (from the desktop file)
    PlasmaCore.SvgItem {
        id: noArtItem
        anchors.fill: parent
        svg: noArtSvg
        visible: !realArt.visible
    }
}

// vi:sts=4:sw=4:et
