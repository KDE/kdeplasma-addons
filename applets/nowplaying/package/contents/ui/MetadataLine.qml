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

    property alias interval: timer.interval
    property Mpris2 source
    property bool showArtist: true
    property bool showAlbum: true

    implicitWidth: metadataLine.implicitWidth
    implicitHeight: metadataLine.implicitHeight

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', function(){
            showArtist = plasmoid.readConfig("displayArtist");
            showAlbum = plasmoid.readConfig("displayAlbum");
        });
    }

    onSourceChanged: {
        if (source) {
            source.titleChanged.connect(function() {
                if (metadataLine.step == 0)
                    metadataLine.updateText();
            });
            source.artistChanged.connect(function() {
                if (metadataLine.step == 1)
                    metadataLine.updateText();
            });
            source.albumChanged.connect(function() {
                if (metadataLine.step == 2)
                    metadataLine.updateText();
            });
        }
    }

    Label {
        id: metadataLine
        anchors {
            // using this instead of parent.fill prevents spurious
            // "possible anchor loop detected on fill" warnings
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        property int step: 0;
        text: source.title
        //fade-out and -in on text change
        Behavior on text {
            SequentialAnimation {
                NumberAnimation { target: metadataLine; property: "opacity"; to: 0 }
                //needs to be here, otherwise the text changes first and fades to itself
                PropertyAction {}
                NumberAnimation { target: metadataLine; property: "opacity"; to: 1 }
            }
        }
        function updateText() {
            var newText = source.title;
            if (metadataLine.step == 1) {
                if (showArtist && source.artist != '') {
                    newText = i18nc("What artist is this track by", "by %1", source.artist);
                } else {
                    metadataLine.step = 2;
                }
            }
            if (metadataLine.step == 2) {
                if (showAlbum && source.album != '') {
                    newText = i18nc("What album is this track on", "on %1", source.album);
                } else {
                    metadataLine.step = 0;
                }
            }
            // prevent fading the text if it hasn't changed
            if (text != newText) {
                text = newText;
            }
        }
        Timer {
            id: timer
            running: true
            interval: 5000
            repeat: true
            onTriggered: {
                metadataLine.step = (metadataLine.step + 1) % 3;
                metadataLine.updateText();
            }
        }
    }
}

// vi:sts=4:sw=4:et
