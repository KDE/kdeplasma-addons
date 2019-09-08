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

import QtQuick 2.6
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.draganddrop 2.0 as DragDrop

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
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
    Layout.minimumWidth: units.iconSizes.medium
    Layout.minimumHeight: units.iconSizes.medium
    Plasmoid.switchWidth: units.gridUnit * 5
    Plasmoid.switchHeight: units.gridUnit * 5

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    // this isn't a frameSVG, the default SVG margins take up around 7% of the frame size, so we use that
    readonly property real horizontalMargins: width * 0.07
    readonly property real verticalMargins: height * 0.07

    // note is of type Note
    property QtObject note: noteManager.loadNote(plasmoid.configuration.noteId);

    // define colors used for icons in ToolButtons and for text in TextArea.
    // this is deliberately _NOT_ the theme color as we are over a known bright background!
    // an unknown colour over a known colour is a bad move as you end up with white on yellow.
    readonly property color textIconColor: {
        if (plasmoid.configuration.color === "black" || plasmoid.configuration.color === "translucent-light") {
            return "#dfdfdf";
        }
        return "#202020";
    }

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

    FocusScope {
        id: focusScope
        anchors {
            fill: parent
            leftMargin: horizontalMargins
            rightMargin: horizontalMargins
            topMargin: verticalMargins
            bottomMargin: verticalMargins
        }

        QQC2.ScrollView {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: fontButtons.top
                bottomMargin: Math.round(units.largeSpacing / 2)
            }

            clip: true

            PlasmaComponents3.TextArea {
                id: mainTextArea

                textFormat: TextEdit.RichText
                onLinkActivated: Qt.openUrlExternally(link)
                background: Rectangle { color: "transparent" }
                color: textIconColor
                persistentSelection: true
                wrapMode: TextEdit.Wrap

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
                        } else if(event.matches(StandardKey.Paste)) {
                            documentHandler.pasteWithoutFormatting();
                            documentHandler.reset();
                            event.accepted = true;
                        } else if(event.matches(StandardKey.Cut)) {
                            mainTextArea.cut();
                            documentHandler.reset();
                            event.accepted = true;
                        }
                    }
                }

                // update the note if the source changes, but only if the user isn't editing it currently
                Binding {
                    target: mainTextArea
                    property: "text"
                    value: note.noteText
                    when: !mainTextArea.activeFocus
                }

                onActiveFocusChanged: {
                    if (activeFocus) {
                        plasmoid.status = PlasmaCore.Types.AcceptingInputStatus
                    } else {
                        plasmoid.status = PlasmaCore.Types.ActiveStatus
                        note.save(mainTextArea.text);
                    }
                }

                onPressed: {
                    if (event.button === Qt.RightButton) {
                        event.accepted = true;
                        contextMenu.popup();
                        mainTextArea.forceActiveFocus();
                    }
                }

                QQC2.Menu {
                    id: contextMenu

                    ShortcutMenuItem {
                        _sequence: StandardKey.Undo
                        _enabled: mainTextArea.canUndo
                        _iconName: "edit-undo"
                        _text: i18n("Undo")
                        onTriggered: contextMenu.retFocus(() => mainTextArea.undo())
                    }

                    ShortcutMenuItem {
                        _sequence: StandardKey.Redo
                        _enabled: mainTextArea.canRedo
                        _iconName: "edit-redo"
                        _text: i18n("Redo")
                        onTriggered: contextMenu.retFocus(() => mainTextArea.redo())
                    }

                    QQC2.MenuSeparator {}

                    ShortcutMenuItem {
                        _sequence: StandardKey.Cut
                        _enabled: mainTextArea.selectedText.length > 0
                        _iconName: "edit-cut"
                        _text: i18n("Cut")
                        onTriggered: contextMenu.retFocus(() => mainTextArea.cut())
                    }

                    ShortcutMenuItem {
                        _sequence: StandardKey.Copy
                        _enabled: mainTextArea.selectedText.length > 0
                        _iconName: "edit-copy"
                        _text: i18n("Copy")
                        onTriggered: contextMenu.retFocus(() => mainTextArea.copy())
                    }

                    ShortcutMenuItem {
                        _sequence: StandardKey.Paste
                        _enabled: mainTextArea.canPaste
                        _iconName: "edit-paste"
                        _text: i18n("Paste Without Formatting")
                        onTriggered: contextMenu.retFocus(() => documentHandler.pasteWithoutFormatting())
                    }

                    ShortcutMenuItem {
                        _enabled: mainTextArea.canPaste
                        _text: i18n("Paste")
                        _iconName: "edit-paste"
                        onTriggered: contextMenu.retFocus(() => mainTextArea.paste())
                    }

                    ShortcutMenuItem {
                        _sequence: StandardKey.Delete
                        _enabled: mainTextArea.selectedText.length > 0
                        _iconName: "edit-delete"
                        _text: i18n("Delete")
                        onTriggered: contextMenu.retFocus(() => mainTextArea.remove(mainTextArea.selectionStart, mainTextArea.selectionEnd))
                    }

                    ShortcutMenuItem {
                        _enabled: mainTextArea.text.length > 0
                        _iconName: "edit-clear"
                        _text: i18n("Clear")
                        onTriggered: contextMenu.retFocus(() => mainTextArea.clear())
                    }

                    QQC2.MenuSeparator {}

                    ShortcutMenuItem {
                        _sequence: StandardKey.SelectAll
                        _enabled: mainTextArea.text.length > 0
                        _iconName: "edit-select-all"
                        _text: i18n("Select All")
                        onTriggered: contextMenu.retFocus(() => mainTextArea.selectAll())
                    }

                    function retFocus(f) {
                        f()
                        documentHandler.reset()
                        mainTextArea.forceActiveFocus()
                    }
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
            }

            readonly property int requiredWidth: formatButtonsRow.width + spacing + settingsButton.width
            readonly property bool showFormatButtons: width > requiredWidth

            Row {
                id: formatButtonsRow
                spacing: units.smallSpacing
                // show format buttons if TextField or any of the buttons have focus
                opacity: fontButtons.showFormatButtons && focusScope.activeFocus ? 1 : 0

                Behavior on opacity { NumberAnimation { duration: units.longDuration } }
                enabled: opacity > 0
                visible: fontButtons.showFormatButtons

                QQC2.ToolButton {
                    icon.name: "format-text-bold"
                    icon.color: textIconColor
                    icon.width: units.iconSizes.smallMedium
                    icon.height: icon.width
                    checked: documentHandler.bold
                    onClicked: documentHandler.bold = !documentHandler.bold
                    Accessible.name: boldTooltip.text
                    QQC2.ToolTip {
                        id: boldTooltip
                        text: i18nc("@info:tooltip", "Bold")
                    }
                }
                QQC2.ToolButton {
                    icon.name: "format-text-italic"
                    icon.color: textIconColor
                    icon.width: units.iconSizes.smallMedium
                    icon.height: icon.width
                    checked: documentHandler.italic
                    onClicked: documentHandler.italic = !documentHandler.italic
                    Accessible.name: italicTooltip.text
                    QQC2.ToolTip {
                        id: italicTooltip
                        text: i18nc("@info:tooltip", "Italic")
                    }
                }
                QQC2.ToolButton {
                    icon.name: "format-text-underline"
                    icon.color: textIconColor
                    icon.width: units.iconSizes.smallMedium
                    icon.height: icon.width
                    checked: documentHandler.underline
                    onClicked: documentHandler.underline = !documentHandler.underline
                    Accessible.name: underlineTooltip.text
                    QQC2.ToolTip {
                        id: underlineTooltip
                        text: i18nc("@info:tooltip", "Underline")
                    }
                }
                QQC2.ToolButton {
                    icon.name: "format-text-strikethrough"
                    icon.color: textIconColor
                    icon.width: units.iconSizes.smallMedium
                    icon.height: icon.width
                    checked: documentHandler.strikeOut
                    onClicked: documentHandler.strikeOut = !documentHandler.strikeOut
                    Accessible.name: strikethroughTooltip.text
                    QQC2.ToolTip {
                        id: strikethroughTooltip
                        text: i18nc("@info:tooltip", "Strikethrough")
                    }
                }
            }

            Item { // spacer
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            QQC2.ToolButton {
                id: settingsButton
                icon.name: "configure"
                icon.color: textIconColor
                icon.width: units.iconSizes.smallMedium
                icon.height: icon.width
                onClicked: plasmoid.action("configure").trigger()
                Accessible.name: settingsTooltip.text
                QQC2.ToolTip {
                    id: settingsTooltip
                    text: plasmoid.action("configure").text
                }
            }
        }
    }

    Component.onCompleted: {
        plasmoid.setAction("change_note_color_white", i18nc("@item:inmenu", "White"));
        plasmoid.setAction("change_note_color_black", i18nc("@item:inmenu", "Black"));
        plasmoid.setAction("change_note_color_red", i18nc("@item:inmenu", "Red"));
        plasmoid.setAction("change_note_color_orange", i18nc("@item:inmenu", "Orange"));
        plasmoid.setAction("change_note_color_yellow", i18nc("@item:inmenu", "Yellow"));
        plasmoid.setAction("change_note_color_green", i18nc("@item:inmenu", "Green"));
        plasmoid.setAction("change_note_color_blue", i18nc("@item:inmenu", "Blue"));
        plasmoid.setAction("change_note_color_pink", i18nc("@item:inmenu", "Pink"));
        plasmoid.setAction("change_note_color_translucent", i18nc("@item:inmenu", "Translucent"));
        plasmoid.setAction("change_note_color_translucent-light", i18nc("@item:inmenu", "Translucent Light"));
        plasmoid.setActionSeparator("separator0");

        // plasmoid configuration doesn't check before emitting change signal
        // explicit check is needed (at time of writing)
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
