/*
 * Copyright 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

Rectangle {
    id: piece
    color: plasmoid.configuration.boardColor
    border.color: "black"
    border.width: 1
    radius: 5
    visible: number != 0

    Layout.minimumWidth: 10
    Layout.preferredWidth: 30

    Layout.minimumHeight: 10
    Layout.preferredHeight: 30

    x: boardColumn * (width + margin) + margin / 2
    y: boardRow * (height + margin) + margin / 2
    width: pieceWidth
    height: pieceHeight

    signal activated(int position)

    property int boardSize: plasmoid.configuration.boardSize
    property int margin: units.smallSpacing
    property int pieceWidth: (parent.width - (margin * boardSize)) / boardSize
    property int pieceHeight: (parent.height - (margin * boardSize)) / boardSize
    property int boardColumn: (position % boardSize)
    property int boardRow: Math.floor(position / boardSize)

    property int number
    property int position

    Behavior on x {
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on y {
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    Components.Label {
        id: pieceNumeral
        anchors.centerIn: parent
        text: piece.number
        color: plasmoid.configuration.numberColor
        visible: plasmoid.configuration.showNumerals
        z: 1
    }

    Image {
        id: pieceImage
        width: parent.width
        height: parent.height
        visible: plasmoid.configuration.useImage
        source: "image://fifteenpuzzle/" + boardSize + "-" + number + "-" + pieceWidth + "-" + pieceHeight + "-" + plasmoid.configuration.imagePath;
        cache: false
        z: 0
    }

    MouseArea {
        anchors.fill: parent
        onClicked: piece.activated(position)
    }
}
