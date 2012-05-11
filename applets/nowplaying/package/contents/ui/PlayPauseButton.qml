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
import "service.js" as Control

Item {
    id: root

    property Mpris2 source;
    implicitWidth: playPauseButton.implicitWidth;
    implicitHeight: playPauseButton.implicitHeight;

    PlasmaComponents.ToolButton {
        id: playPauseButton
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        property string operation: (source.playbackStatus == 'Playing' ? 'Pause' : 'Play')
        iconSource: (source.playbackStatus == 'Playing' ? "media-playback-pause" : "media-playback-start")
        onClicked: {
            Control.callCommand(operation);
        }
        Component.onCompleted: {
            Control.associateItem(playPauseButton, operation);
        }
        onOperationChanged: {
            Control.associateItem(playPauseButton, operation);
        }
    }
}

// vi:sts=4:sw=4:et
