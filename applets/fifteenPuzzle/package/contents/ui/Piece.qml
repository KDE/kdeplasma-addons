/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

Rectangle {
    id: piece
    color: Plasmoid.configuration.boardColor
    border.color: "black"
    border.width: 1
    radius: 5
    visible: !empty

    Layout.minimumWidth: 10
    Layout.preferredWidth: 30

    Layout.minimumHeight: 10
    Layout.preferredHeight: 30

    x: boardColumn * (width + margin) + margin / 2
    y: boardRow * (height + margin) + margin / 2
    width: pieceWidth
    height: pieceHeight

    signal activated(int position)

    readonly property int boardSize: Plasmoid.configuration.boardSize
    readonly property int margin: PlasmaCore.Units.smallSpacing
    readonly property int pieceWidth: (parent.width - (margin * boardSize)) / boardSize
    readonly property int pieceHeight: (parent.height - (margin * boardSize)) / boardSize
    readonly property int boardColumn: (position % boardSize)
    readonly property int boardRow: Math.floor(position / boardSize)
    readonly property bool empty: number === 0

    property int number
    property int position

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            tapHandler.tapped(null);
            break;
        }
    }
    Accessible.name: pieceNumeral.text
    Accessible.role: Accessible.Button

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

    Loader {
        anchors.fill: parent
        active: parent.activeFocus
        asynchronous: true
        z: 0

        sourceComponent: PlasmaExtras.Highlight {
            hovered: true
        }
    }

    PlasmaComponents3.Label {
        id: pieceNumeral
        anchors.centerIn: parent
        text: piece.number
        color: Plasmoid.configuration.numberColor
        visible: Plasmoid.configuration.showNumerals
        z: 1
    }

    Loader {
        anchors.fill: parent

        active: Plasmoid.configuration.useImage
        asynchronous: true
        z: 0

        sourceComponent: Image {
            id: pieceImage
            source: "image://fifteenpuzzle/" + boardSize + "-" + number + "-" + pieceWidth + "-" + pieceHeight + "-" + Plasmoid.configuration.imagePath
            cache: false
        }
    }

    TapHandler {
        id: tapHandler

        onTapped: {
            piece.forceActiveFocus();
            piece.activated(position);
        }
    }
}
