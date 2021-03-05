/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

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

    readonly property int boardSize: plasmoid.configuration.boardSize
    readonly property int margin: PlasmaCore.Units.smallSpacing
    readonly property int pieceWidth: (parent.width - (margin * boardSize)) / boardSize
    readonly property int pieceHeight: (parent.height - (margin * boardSize)) / boardSize
    readonly property int boardColumn: (position % boardSize)
    readonly property int boardRow: Math.floor(position / boardSize)

    property int number
    property int position

    Behavior on x {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on y {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    PlasmaComponents3.Label {
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
