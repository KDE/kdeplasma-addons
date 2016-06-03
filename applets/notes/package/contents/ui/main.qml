/*
    Copyright (C) 2014 David Edmundson <davidedmundson@kde.org>
    Copyright (C) 2014, 2015 Kai Uwe Broulik <kde@privat.broulik.de>

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
import QtQuick.Controls 1.3 as QtControls
import QtQuick.Layouts 1.1

import org.kde.draganddrop 2.0 as DragDrop

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

// so we can override the textColor...
import QtQuick.Controls.Styles.Plasma 2.0 as PlasmaStyle

import org.kde.plasma.private.notes 0.1

PlasmaCore.SvgItem {
    id: root

    svg: PlasmaCore.Svg {
        imagePath: "widgets/notes"
    }
    elementId: plasmoid.configuration.color + "-notes"

    width: units.gridUnit * 14
    height: units.gridUnit * 14
    Layout.minimumWidth: units.gridUnit * 4
    Layout.minimumHeight: units.gridUnit * 4
    Plasmoid.switchWidth: Math.max(units.gridUnit * 13, fontButtons.Layout.preferredWidth)
    Plasmoid.switchHeight: units.gridUnit * 5

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    //this isn't a frameSVG, the default SVG margins take up around 7% of the frame size, so we use that
    property real horizontalMargins: width * 0.07
    property real verticalMargins: height * 0.07

    //note is of type Note
    property QtObject note: noteManager.loadNote(plasmoid.configuration.noteId);

    Timer {
        id: forceFocusTimer
        interval: 1
        onTriggered: mainTextArea.forceActiveFocus()
    }

    Connections {
        target: plasmoid
        onExpandedChanged: {
            // don't autofocus when we're on the desktop
            if (expanded && (plasmoid.formFactor === PlasmaCore.Types.Vertical || plasmoid.formFactor === PlasmaCore.Types.Horizontal)) {
                mainTextArea.forceActiveFocus()
            }
        }
        onActivated: {
            // FIXME doing forceActiveFocus here directly doesn't work
            forceFocusTimer.restart()
        }
        onExternalData: {
            // if we dropped a text file, we want its contents,
            // otherwise we take the external data verbatim
            var contents = NotesHelper.fileContents(data) || data
            mainTextArea.text = contents.replace(/\n/g, "<br>") // what about richtext?

            // place cursor at the end of text, there's no "just move the cursor" function
            mainTextArea.moveCursorSelection(mainTextArea.length)
            mainTextArea.deselect()
        }
    }

    NoteManager {
        id: noteManager
    }

    Plasmoid.compactRepresentation: DragDrop.DropArea {
        id: compactDropArea
        onDragEnter: activationTimer.restart()
        onDragLeave: activationTimer.stop()

        Timer {
            id: activationTimer
            interval: 250 // matches taskmanager delay
            onTriggered: plasmoid.expanded = true
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: plasmoid.expanded = !plasmoid.expanded

            PlasmaCore.IconItem {
                anchors.fill: parent
                source: "knotes"
                colorGroup: PlasmaCore.ColorScope.colorGroup
                active: parent.containsMouse
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

    PlasmaComponents.TextArea {
        id: mainTextArea
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

        backgroundVisible: false
        frameVisible: false
        textFormat: TextEdit.RichText

        onLinkActivated: Qt.openUrlExternally(link)

        Keys.onPressed: {
            if(event.key === Qt.Key_Escape) {
                plasmoid.expanded = false;
                event.accepted = true;
            } else if(event.modifiers === Qt.ControlModifier) {
                if(event.key === Qt.Key_B) {
                    documentHandler.bold = !documentHandler.bold;
                    event.accepted = true;
                } else if(event.key === Qt.Key_I) {
                    documentHandler.italic = !documentHandler.italic;
                    event.accepted = true;
                } else if(event.key === Qt.Key_U) {
                    documentHandler.underline = !documentHandler.underline;
                    event.accepted = true;
                } else if(event.key === Qt.Key_S) {
                    documentHandler.strikeOut = !documentHandler.strikeOut;
                    event.accepted = true;
                }
            }
        }

        style: PlasmaStyle.TextAreaStyle {
            //this is deliberately _NOT_ the theme color as we are over a known bright background
            //an unknown colour over a known colour is a bad move as you end up with white on yellow
            textColor: plasmoid.configuration.color === "black" ? "#dfdfdf" : "#202020"
        }

        //update the note if the source changes, but only if the user isn't editing it currently
        Binding {
            target: mainTextArea
            property: "text"
            value: note.noteText
            when: !mainTextArea.activeFocus
        }

        onActiveFocusChanged: {
            if (!activeFocus) {
                note.save(mainTextArea.text);
            }
        }
    }

    DragDrop.DropArea {
        id: dropArea
        anchors.fill: mainTextArea

        function positionOfDrop(event) {
            return mainTextArea.positionAt(event.x, event.y + mainTextArea.flickableItem.contentY)
        }

        onDrop: {
            var mimeData = event.mimeData
            var text = ""
            if (mimeData.hasUrls) {
                var urls = mimeData.urls
                for (var i = 0, j = urls.length; i < j; ++i) {
                    var url = urls[i]
                    text += "<a href=\"" + url + "\">" + url + "</a><br>"
                }
            } else {
                text = mimeData.text.replace(/\n/g, "<br>")
            }

            mainTextArea.insert(positionOfDrop(event), text)
            event.accept(Qt.CopyAction)
        }
        onDragMove: {
            // there doesn't seem to be a "just move the cursor", so we move
            // the selection and then unselect so the cursor follows the mouse
            mainTextArea.moveCursorSelection(positionOfDrop(event))
            mainTextArea.deselect()
        }
        onDragEnter: mainTextArea.forceActiveFocus()
    }

    RowLayout {
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
            tooltip: i18n("Toggle text format options")
            iconSource: "draw-text"
            checkable: true
            Accessible.name: tooltip
        }

        PlasmaComponents.ToolButton {
            tooltip: i18n("Bold")
            iconSource: "format-text-bold"
            opacity: toggleFormatBarButton.checked ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: units.longDuration } }
            enabled: opacity > 0

            checked: documentHandler.bold
            onClicked: documentHandler.bold = !documentHandler.bold
            Accessible.name: tooltip
        }
        PlasmaComponents.ToolButton {
            tooltip: i18n("Italic")
            iconSource: "format-text-italic"
            opacity: toggleFormatBarButton.checked ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: units.longDuration } }
            enabled: opacity > 0

            checked: documentHandler.italic
            onClicked: documentHandler.italic = !documentHandler.italic
            Accessible.name: tooltip
        }
        PlasmaComponents.ToolButton {
            tooltip: i18n("Underline")
            iconSource: "format-text-underline"
            opacity: toggleFormatBarButton.checked ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: units.longDuration } }
            enabled: opacity > 0

            checked: documentHandler.underline
            onClicked: documentHandler.underline = !documentHandler.underline
            Accessible.name: tooltip
        }
        PlasmaComponents.ToolButton {
            tooltip: i18n("Strikethrough")
            iconSource: "format-text-strikethrough"
            opacity: toggleFormatBarButton.checked ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: units.longDuration } }
            enabled: opacity > 0

            checked: documentHandler.strikeOut
            onClicked: documentHandler.strikeOut = !documentHandler.strikeOut
            Accessible.name: tooltip
        }

        Item { // spacer
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        PlasmaComponents.ToolButton {
            tooltip: i18n("Notes Settings...")
            iconSource: "configure"
            onClicked: plasmoid.action("configure").trigger()
            Accessible.name: tooltip
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
