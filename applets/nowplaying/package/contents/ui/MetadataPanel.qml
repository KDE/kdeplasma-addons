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

    property Mpris2 source
    property int contentLeftOffset: Math.max(byLabel.width, onLabel.width) + 3
    property bool showArtist: true
    property bool showAlbum: true

    implicitHeight: childrenRect.height
    implicitWidth: contentLeftOffset + Math.max(Math.max(titleLabel.implicitWidth, artistLabel.implicitWidth), albumLabel.implicitWidth)
    height: childrenRect.height

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', function(){
            showArtist = plasmoid.readConfig("displayArtist");
            showAlbum = plasmoid.readConfig("displayAlbum");
        });
    }

    Label {
        id: titleLabel
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: contentLeftOffset
            right: parent.right
        }
        text: source.title
    }
    Label {
        id: artistLabel
        anchors {
            top: titleLabel.bottom
            left: parent.left
            leftMargin: contentLeftOffset
            right: parent.right
        }
        visible: showArtist && text != ''
        text: source.artist
    }
    Label {
        id: albumLabel
        anchors {
            top: artistLabel.visible ? artistLabel.bottom : titleLabel.bottom
            left: parent.left
            leftMargin: contentLeftOffset
            right: parent.right
        }
        visible: showAlbum && text != ''
        text: source.album
    }
    Label {
        id: byLabel
        anchors {
            right: artistLabel.left
            rightMargin: 3
            baseline: artistLabel.baseline
        }
        visible: artistLabel.visible
        text: i18nc("What artist is this track by", "by")
        font.weight: Font.Light
    }
    Label {
        id: onLabel
        anchors {
            right: albumLabel.left
            rightMargin: 3
            baseline: albumLabel.baseline
        }
        visible: albumLabel.visible
        text: i18nc("What album is this track on", "on")
        font.weight: Font.Light
    }
}

// vi:sts=4:sw=4:et
