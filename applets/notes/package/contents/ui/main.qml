/*
    Copyright (C) 2014 David Edmundson <davidedmundson@kde.org>
    Copyright (C) 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.notes 0.1

PlasmaCore.SvgItem
{
    svg: PlasmaCore.Svg {
        imagePath: "widgets/notes"
    }
    elementId: plasmoid.configuration.color + "-notes"

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    //this isn't a frameSVG, the default SVG margins take up around 7% of the frame size, so we use that
    property real horizontalMargins: width * 0.07
    property real verticalMargins: height * 0.07

    //note is of type Note
    property QtObject note: noteManager.loadNote(plasmoid.configuration.noteId);

    NoteManager {
        id: noteManager
    }

    PlasmaExtras.ScrollArea {
        id: mainScrollArea
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            leftMargin: horizontalMargins
            rightMargin: horizontalMargins
            topMargin: verticalMargins

            bottom: fontButtons.top
            bottomMargin: Math.round(units.largeSpacing / 2)
        }

        Flickable {
            id: flickable
            contentHeight: mainTextArea.height

            // TextEdit doesn't handle scrolling while typing itself
            function ensureVisible(rect) {
                if (contentY >= rect.y) {
                    contentY = rect.y
                } else if (contentY + height < rect.y + rect.height) {
                    contentY = rect.y + rect.height - height
                }
            }

            //update the note if the source changes, but only if the user isn't editing it currently
            Binding {
                target: mainTextArea
                property: "text"
                value: note.noteText
                when: !mainTextArea.activeFocus
            }

            //deliberately not PlasmaComponents.textEdit
            //as we have custom font selection
            TextEdit {
                id: mainTextArea
                width: parent.width
                height: Math.max(mainScrollArea.height, paintedHeight)
                onCursorRectangleChanged: flickable.ensureVisible(cursorRectangle)
                focus: true
                textFormat: Qt.RichText
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                renderType: Text.NativeRendering

                //this is deliberately _NOT_ the theme colour as we are over a known bright background
                //an unknown colour over a known colour is a bad move as you end up with white on yellow
                color: "#202020"
                selectedTextColor: theme.viewBackgroundColor
                selectionColor: theme.viewFocusColor

                font.capitalization: theme.defaultFont.capitalization
                font.family: theme.defaultFont.family
                font.italic: documentHandler.italic
                font.letterSpacing: theme.defaultFont.letterSpacing
                font.pointSize: theme.defaultFont.pointSize
                font.strikeout: theme.defaultFont.strikeout
                font.underline: theme.defaultFont.underline
                font.weight: theme.defaultFont.weight
                font.wordSpacing: theme.defaultFont.wordSpacing

                onActiveFocusChanged: {
                    if (!activeFocus) {
                        note.save(mainTextArea.text);
                    }
                }
            }
        }
    }

     DocumentHandler {
        id: documentHandler
        target: mainTextArea
        cursorPosition: mainTextArea.cursorPosition
        selectionStart: mainTextArea.selectionStart
        selectionEnd: mainTextArea.selectionEnd
    }

    Row {
        id: fontButtons
        spacing: units.smallSpacing
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right

            leftMargin: horizontalMargins
            rightMargin: horizontalMargins
            bottomMargin: verticalMargins
        }
        PlasmaComponents.ToolButton {
            id: toggleFormatBarButton
            iconSource: "draw-text"
            checkable: true
        }

        PlasmaComponents.ToolButton {
            id: formatBoldButton
            iconSource: "format-text-bold"
            visible: toggleFormatBarButton.checked
            checkable: true
            checked: documentHandler.bold
            onClicked: documentHandler.bold = !documentHandler.bold

        }
        PlasmaComponents.ToolButton {
            iconSource: "format-text-italic"
            visible: toggleFormatBarButton.checked
            checkable: true
            checked: documentHandler.italic
            onClicked: documentHandler.italic = !documentHandler.italic
        }
        PlasmaComponents.ToolButton {
            iconSource: "format-text-underline"
            visible: toggleFormatBarButton.checked
            checkable: true
            checked: documentHandler.underline
            onClicked: documentHandler.underline = !documentHandler.underline
        }
        PlasmaComponents.ToolButton {
            iconSource: "format-text-strikethrough"
            visible: toggleFormatBarButton.checked
            checkable: true
            checked: documentHandler.strikeOut
            onClicked: documentHandler.strikeOut = !documentHandler.strikeOut
        }
        PlasmaComponents.ToolButton {
            iconSource: "format-justify-center"
            visible: toggleFormatBarButton.checked
            checkable: true

        }
        PlasmaComponents.ToolButton {
            iconSource: "format-justify-fill"
            visible: toggleFormatBarButton.checked
            checkable: true
        }
    }

    Component.onCompleted: {
        plasmoid.setAction("change_note_color_white", i18n("White"));
        plasmoid.setAction("change_note_color_black", i18n("Black"));
        plasmoid.setAction("change_note_color_red", i18n("Red"));
        plasmoid.setAction("change_note_color_orange", i18n("Orange"));
        plasmoid.setAction("change_note_color_yellow", i18n("Yellow"));
        plasmoid.setAction("change_note_color_green", i18n("Green"));
        plasmoid.setAction("change_note_color_blue", i18n("Blue"));
        plasmoid.setAction("change_note_color_pink", i18n("Pink"));
        plasmoid.setAction("change_note_color_translucent", i18n("Translucent"));
        plasmoid.setActionSeparator("separator0");

        //plasmoid configuration doesn't check before emitting change signal
        //explicit check is needed (at time of writing)
        if (note.id != plasmoid.configuration.noteId) {
            plasmoid.configuration.noteId = note.id;
        }
    }

    Component.onDestruction: {
        note.save(mainTextArea.text);
    }

    function actionTriggered(actionName) {
        if (actionName.indexOf("change_note_color_") == 0){
            plasmoid.configuration.color = actionName.replace("change_note_color_", "");
        }
    }
}
