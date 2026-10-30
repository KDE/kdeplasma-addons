/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

Rectangle {
    id: piece
    color: Plasmoid.configuration.boardColor
    border.color: "black"
    border.width: 1
    radius: Kirigami.Units.cornerRadius
    visible: !empty

    x: boardColumn * (width + margin) + margin / 2
    y: boardRow * (height + margin) + margin / 2
    width: pieceWidth
    height: pieceHeight

    signal activated(int position)

    readonly property int boardSize: Plasmoid.configuration.boardSize
    readonly property int margin: Kirigami.Units.smallSpacing
    readonly property int pieceWidth: (parent.width - (margin * boardSize)) / boardSize
    readonly property int pieceHeight: (parent.height - (margin * boardSize)) / boardSize
    readonly property int boardColumn: (position % boardSize)
    readonly property int boardRow: Math.floor(position / boardSize)
    readonly property bool empty: number === 0

    property int number
    property int position
    property bool animationEnabled

    Keys.onPressed: event => {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            piece.trigger();
            break;
        }
    }
    Accessible.name: pieceNumeral.text
    Accessible.role: Accessible.Button

    function trigger() {
        piece.forceActiveFocus();
        piece.activated(position);
    }

    Behavior on x {
        enabled: piece.animationEnabled
        NumberAnimation {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on y {
        enabled: piece.animationEnabled
        NumberAnimation {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    TapHandler {
        onTapped: piece.trigger()
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
        textFormat: Text.PlainText
        color: Plasmoid.configuration.numberColor
        visible: Plasmoid.configuration.showNumerals || !Plasmoid.configuration.useImage
        z: 1
    }

    Loader {
        anchors.fill: parent

        active: Plasmoid.configuration.useImage
        asynchronous: true
        z: 0

        sourceComponent: Image {
            id: pieceImage
            source: (piece.pieceWidth > 0 && piece.pieceHeight > 0 && piece.number > 0) ? "image://fifteenpuzzle/" + piece.boardSize + "-" + piece.number + "-" + piece.pieceWidth + "-" + piece.pieceHeight + "-" + Plasmoid.configuration.imagePath : ""
            cache: false
        }
    }
}
