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
import org.kde.kcoreaddons 1.0 as KCoreAddons

Item {
    id: main

    Layout.minimumWidth: Math.max(boardSize * 10, controlsRow.width)
    Layout.preferredWidth: Math.max(boardSize * 10, controlsRow.width)

    property int boardSize: plasmoid.configuration.boardSize
    property Component piece: Piece {}
    property var pieces: []

    property int seconds: 0

    function fillBoard() {
        // Clear out old board
        for (var i = 0; i < pieces.length; ++i)  {
            pieces[i].destroy();
        }

        pieces = [];
        var size = boardSize * boardSize;
        if (piece.status == Component.Ready) {
            for (var i = 0; i < size; ++i) {
                var newPiece = piece.createObject(mainGrid, {"number": i, "position": i });
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
        secondsTimer.start();

        var size = boardSize * boardSize;
        for (var i = size - 1; i >= 0; --i) {
            // choose a random number such that 0 <= rand <= i
            var rand = Math.floor(Math.random() * 10) % (i + 1);
            swapPieces(i, rand);
        }

        // make sure the new board is solveable

        // count the number of inversions
        // an inversion is a pair of tiles at positions a, b where
        // a < b but value(a) > value(b)

        // also count the number of lines the blank tile is from the bottom
        var inversions = 0;
        var blankRow = -1;
        for (var i = 0; i < size; ++i) {
            if (pieces[i].number == 0) {
                blankRow = i / boardSize;
                continue;
            }
            for (var j = 0; j < i; ++j) {
                if (pieces[j].number == 0) {
                    continue;
                }
                if (pieces[i].number < pieces[j].number) {
                    ++inversions;
                }
            }
        }

        if (blankRow == -1) {
            console.log("Unable to find row of blank tile");
        }

        // we have a solveable board if:
        // size is odd:  there are an even number of inversions
        // size is even: the number of inversions is odd if and only if
        //               the blank tile is on an odd row from the bottom-
        var sizeMod2 = Math.floor(boardSize % 2);
        var inversionsMod2 = Math.floor(inversions % 2);
        var solveable = (sizeMod2 == 1 && inversionsMod2 == 0) ||
                         (sizeMod2 == 0 && inversionsMod2 == 0) == (Math.floor((boardSize - blankRow) % 2) == 1);
        if (!solveable) {
            // make the grid solveable by swapping two adjacent pieces around
            var pieceA = 0;
            var pieceB = 1;
            if (pieces[pieceA].number == 0) {
                pieceA = boardSize + 1;
            } else if (pieces[pieceB].number == 0) {
                pieceB = boardSize;
            }
            swapPieces(pieceA, pieceB);
        }
    }

    function pieceClicked(position) {
        // If the position is next above, below, right or left of the piece 0, swap them
        var left = (position % boardSize) > 0 ? position - 1 : -1;
        var right = (position % boardSize) < (boardSize - 1) ? position + 1 : -1;
        var above = Math.floor(position / boardSize) > 0 ? position - boardSize : -1;
        var below = Math.floor(position / boardSize) < (boardSize - 1) ? position + boardSize : -1;
        if (left != -1 && pieces[left].number == 0) {
            swapPieces(left, position);
        } else if (right != -1 && pieces[right].number == 0) {
            swapPieces(right, position);
        } else if (above != -1 && pieces[above].number == 0) {
            swapPieces(above, position);
        } else if (below != -1 && pieces[below].number == 0) {
            swapPieces(below, position);
        }
        checkSolved();
    }

    function checkSolved() {
        var size = boardSize * boardSize;
        for (var i = 0; i < size - 1; ++i) {
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
        var firstPiece = pieces[first];
        var secondPiece = pieces[second];
        var temp = firstPiece.position;
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
        color: theme.backgroundColor
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: controlsRow.top
            bottomMargin: units.smallSpacing
        }
    }

    RowLayout {
        id: controlsRow
        anchors {
            margins: units.smallSpacing
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        Components.Button {
            id: button
            Layout.fillWidth: true
            iconName: "roll"
            text: i18n("Shuffle");
            onClicked: main.shuffleBoard();
        }

        Components.Label {
            id: timeLabel
            Layout.fillWidth: true
            text: main.timerText()
            color: theme.textColor
        }
    }

    Rectangle {
        id: solvedRect
        visible: false
        anchors.fill: mainGrid
        color: theme.backgroundColor
        z: 0

        Image {
            id: solvedImage
            anchors.fill: parent
            z: 1
            source: "image://fifteenpuzzle/" + boardSize + "-all-0-0-" + plasmoid.configuration.imagePath;
            visible: plasmoid.configuration.useImage;
            cache: false
            function update() {
                var tmp = source;
                source = "";
                source = tmp;
            }
        }

        Components.Label {
            id: solvedLabel
            anchors.centerIn: parent
            color: theme.textColor
            text: i18n("Solved! Try again.")
            z: 2
        }
    }

    Timer {
        id: secondsTimer
        interval: 1000
        repeat: true

        onTriggered: ++main.seconds;
    }

    Connections {
        target: plasmoid.configuration
        onBoardSizeChanged: {
            main.fillBoard();
            solvedImage.update();
        }
    }

    Connections {
        target: plasmoid.configuration
        onImagePathChanged: {
            main.fillBoard();
            solvedImage.update();
        }
    }

    Component.onCompleted: {
        main.fillBoard();
        solvedImage.update();
    }
}
