/*
 *   Copyright 2012 Alex Merry <alex.merry@kdemail.net>
 *
 *   Code taken from PlasmaComponents.ProgressBar:
 *   Copyright 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
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
import org.kde.qtextracomponents 0.1
import "service.js" as Control

Item {
    id: root
    property Mpris2 source
    property int orientation: Qt.Horizontal

    implicitHeight: 10
    implicitWidth: 100

    onSourceChanged: {
        if (source) {
            source.dataChanged.connect(function() {
                range.update();
            });
        }
    }

    PlasmaCore.Theme {
        id: theme
    }

    Timer {
        id: updateTimer

        interval: 500
        running: source.playbackStatus == 'Playing'
        repeat: true
        onTriggered: {
            range.update();
        }
    }
    PlasmaComponents.RangeModel {
        id: range

        // default values
        minimumValue: 0.0
        maximumValue: source.trackLength
        value: 0
        function update() {
            value = source.currentPos();
        }

        positionAtMinimum: 0
        positionAtMaximum: contents.width
    }

    Rectangle {
        id: contents

        property bool _isVertical: orientation == Qt.Vertical

        width: _isVertical ? root.height : root.width
        height: _isVertical ? root.width : root.height
        rotation: _isVertical ? 90 : 0
        anchors.centerIn: parent

        Rectangle {
            id: barBg
            anchors.fill: parent
            color: theme.highlightColor
            opacity: 0.4
        }
        Rectangle {
            id: bar
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            color: theme.highlightColor
            visible: range.position > 0
            width: range.position
        }
        MouseArea {
            id: seekArea
            anchors.fill: parent
            onClicked: {
                var x = mouse.x;
                Control.seek(range.valueForPosition(x));
            }
        }
    }
}

// vi:sts=4:sw=4:et
