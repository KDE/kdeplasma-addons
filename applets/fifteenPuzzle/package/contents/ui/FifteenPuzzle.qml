/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

Item {
    id: main

    Layout.minimumWidth: Math.max(boardSize * 10, controlsRow.width)
    Layout.preferredWidth: Math.max(boardSize * 10, controlsRow.width)

    readonly property int boardSize: Plasmoid.configuration.boardSize
    property Component piece: Piece {}
    property var pieces: []

    property int seconds: 0

    function fillBoard() {
        // Clear out old board
        for (const piece of pieces) {
            piece.destroy();
        }

        pieces = [];
        const count = boardSize * boardSize;
        if (piece.status === Component.Ready) {
            for (let i = 0; i < count; ++i) {
                const newPiece = piece.createObject(mainGrid, {"number": i, "position": i });
                pieces[i] = newPiece;
                newPiece.activated.connect(pieceClicked);
            }
            shuffleBoard();
        }
    }

    function shuffleBoard() {
        // Hide the solved rectangle in case it was visible
        solvedRect.visible = false;
        main.seconds = 0;

        const count = boardSize * boardSize;
        for (let i = count - 1; i >= 0; --i) {
            // choose a random number such that 0 <= rand <= i
            const rand = Math.floor(Math.random() * 10) % (i + 1);
            swapPieces(i, rand);
        }

        // make sure the new board is solveable

        // count the number of inversions
        // an inversion is a pair of tiles at positions a, b where
        // a < b but value(a) > value(b)

        // also count the number of lines the blank tile is from the bottom
        let inversions = 0;
        let blankRow = -1;
        for (let i = 0; i < count; ++i) {
            if (pieces[i].number === 0) {
                blankRow = Math.floor(i / boardSize);
                continue;
            }
            for (let j = 0; j < i; ++j) {
                if (pieces[j].number === 0) {
                    continue;
                }
                if (pieces[i].number < pieces[j].number) {
                    ++inversions;
                }
            }
        }

        if (blankRow === -1) {
            console.log("Unable to find row of blank tile");
        }

        // we have a solveable board if:
        // size is odd:  there are an even number of inversions
        // size is even: the number of inversions is odd if and only if
        //               the blank tile is on an odd row from the bottom-
        const sizeMod2 = Math.floor(boardSize % 2);
        const inversionsMod2 = Math.floor(inversions % 2);
        const solveable = (sizeMod2 === 1 && inversionsMod2 === 0) ||
                          (sizeMod2 === 0 && (inversionsMod2 === 0) === (Math.floor((boardSize - blankRow) % 2) === 1));
        if (!solveable) {
            // make the grid solveable by swapping two adjacent pieces around
            let pieceA = 0;
            let pieceB = 1;
            if (pieces[pieceA].number === 0) {
                pieceA = boardSize + 1;
            } else if (pieces[pieceB].number === 0) {
                pieceB = boardSize;
            }
            swapPieces(pieceA, pieceB);
        }
        secondsTimer.stop();
    }

    function pieceClicked(position) {
        // If the position is next above, below, right or left of the piece 0, swap them
        const left = (position % boardSize) > 0 ? position - 1 : -1;
        const right = (position % boardSize) < (boardSize - 1) ? position + 1 : -1;
        const above = Math.floor(position / boardSize) > 0 ? position - boardSize : -1;
        const below = Math.floor(position / boardSize) < (boardSize - 1) ? position + boardSize : -1;
        if (left !== -1 && pieces[left].number === 0) {
            swapPieces(left, position);
        } else if (right !== -1 && pieces[right].number === 0) {
            swapPieces(right, position);
        } else if (above !== -1 && pieces[above].number === 0) {
            swapPieces(above, position);
        } else if (below !== -1 && pieces[below].number === 0) {
            swapPieces(below, position);
        }
        secondsTimer.start();
        checkSolved();
    }

    function checkSolved() {
        const count = boardSize * boardSize;
        for (let i = 0; i < count - 2; ++i) {
            if (pieces[i].number > pieces[i + 1].number) {
                // Not solved.
                return;
            }
        }
        solved();
    }

    function solved() {
        // Show a message that it was solved.
        console.log("Puzzle was solved");
        solvedRect.visible = true;
        // Stop the timer
        secondsTimer.stop();
    }

    function swapPieces(first, second) {
        const firstPiece = pieces[first];
        const secondPiece = pieces[second];
        let temp = firstPiece.position;
        firstPiece.position = secondPiece.position;
        secondPiece.position = temp;
        temp = pieces[first];
        pieces[first] = pieces[second];
        pieces[second] = temp;
    }

    function timerText() {
        return i18nc("The time since the puzzle started, in minutes and seconds",
                     "Time: %1", KCoreAddons.Format.formatDuration(seconds * 1000, KCoreAddons.FormatTypes.FoldHours));
    }

    Rectangle {
        id: mainGrid
        color: PlasmaCore.Theme.backgroundColor
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: controlsRow.top
            bottomMargin: PlasmaCore.Units.smallSpacing
        }
    }

    RowLayout {
        id: controlsRow
        anchors {
            margins: PlasmaCore.Units.smallSpacing
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        PlasmaComponents3.Button {
            id: button
            Layout.fillWidth: true
            icon.name: "roll"
            text: i18nc("@action:button", "Shuffle");
            onClicked: main.shuffleBoard();
        }

        PlasmaComponents3.Label {
            id: timeLabel
            Layout.fillWidth: true
            text: main.timerText()
            color: PlasmaCore.Theme.textColor
        }
    }

    Rectangle {
        id: solvedRect
        visible: false
        anchors.fill: mainGrid
        color: PlasmaCore.Theme.backgroundColor
        z: 0

        Image {
            id: solvedImage
            anchors.fill: parent
            z: 1
            source: "image://fifteenpuzzle/" + boardSize + "-all-0-0-" + Plasmoid.configuration.imagePath
            visible: Plasmoid.configuration.useImage
            cache: false
            function update() {
                const tmp = source;
                source = "";
                source = tmp;
            }
        }

        PlasmaComponents3.Label {
            id: solvedLabel
            anchors.centerIn: parent
            color: PlasmaCore.Theme.textColor
            text: i18nc("@info", "Solved! Try again.")
            z: 2
        }
    }

    Timer {
        id: secondsTimer
        interval: 1000
        repeat: true

        onTriggered: ++main.seconds
    }

    Connections {
        target: Plasmoid.configuration
        function onBoardSizeChanged() {
            main.fillBoard();
            solvedImage.update();
        }
    }

    Connections {
        target: Plasmoid.configuration
        function onImagePathChanged() {
            main.fillBoard();
            solvedImage.update();
        }
    }

    Component.onCompleted: {
        main.fillBoard();
        solvedImage.update();
    }
}
