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

    property int minimumWidth: minimumHeight + 120
    property int minimumHeight: metadataLine.implicitHeight + progressBar.height + metadataLine.anchors.bottomMargin

    Mpris2 {
        id: source
    }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent

        AlbumArt {
            id: albumArt
            artUrl: source.artUrl
            anchors {
                top: parent.top
                left: parent.left
                bottom: parent.bottom
            }
            width: height
        }

        MetadataLine {
            id: metadataLine
            anchors {
                top: parent.top
                left: albumArt.right
                leftMargin: 2
                right: parent.right
                bottom: progressBar.top
                bottomMargin: 3
            }
            interval: 5000
            source: source
        }

        PositionSlider {
            id: progressBar
            anchors {
                bottom: parent.bottom
                left: albumArt.right
                leftMargin: 2
                right: parent.right
            }
            height: 5
            source: source
        }

        PlayPauseButton {
            id: ppButton
            visible: source.canControl && mouseArea.containsMouse
            width: height
            anchors.fill: albumArt
            source: source
        }
    }
}

// vi:sts=4:sw=4:et
