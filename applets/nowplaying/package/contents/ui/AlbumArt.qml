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
    property Mpris2 source;

    implicitWidth: {
        if (realArt.visible) {
            realArt.implicitWidth;
        } else if (logoArtItem.visible) {
            logoArtItem.implicitWidth
        } else {
            noArtItem.implicitWidth
        }
    }
    implicitHeight: {
        if (realArt.visible) {
            realArt.implicitHeight;
        } else if (logoArtItem.visible) {
            logoArtItem.implicitHeight
        } else {
            noArtItem.implicitHeight
        }
    }


    PlasmaCore.Svg {
        id: noArtSvg
        imagePath: plasmoid.file("images", "nocover.svgz")
    }
    // TODO: figure out how to centre this vertically
    PlasmaCore.SvgItem {
        id: noArtItem
        anchors.fill: parent
        svg: noArtSvg
        visible: !realArt.visible && !logoArtItem.visible
    }
    PlasmaCore.IconItem {
        id: logoArtItem
        anchors.fill: parent
        visible: !realArt.visible && root.source.logo != ''
        source: root.source.logo
    }
    Image {
        id: realArt
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        visible: (status == Image.Ready)
        source: root.source.artUrl
    }
}

// vi:sts=4:sw=4:et
