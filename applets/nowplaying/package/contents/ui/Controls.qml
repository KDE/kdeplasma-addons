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
import "../code/service.js" as Control

Row {
    id: root

    property Mpris2 source;

    PlasmaComponents.ToolButton {
        id: playPauseButton
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
    PlasmaComponents.ToolButton {
        id: stopButton
        iconSource: "media-playback-stop"
        onClicked: {
            Control.callCommand('Stop');
        }
        Component.onCompleted: {
            Control.associateItem(stopButton, 'Stop');
        }
    }
    PlasmaComponents.ToolButton {
        id: prevButton
        iconSource: "media-skip-backward"
        onClicked: {
            Control.callCommand('Previous');
        }
        Component.onCompleted: {
            Control.associateItem(prevButton, 'Previous');
        }
    }
    PlasmaComponents.ToolButton {
        id: nextButton
        iconSource: "media-skip-forward"
        onClicked: {
            Control.callCommand('Next');
        }
        Component.onCompleted: {
            Control.associateItem(nextButton, 'Next');
        }
    }
}

// vi:sts=4:sw=4:et
