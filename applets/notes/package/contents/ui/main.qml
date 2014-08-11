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

import org.kde.plasma.private.notes 0.1 as Notes

PlasmaCore.SvgItem
{
    svg: PlasmaCore.Svg {
        imagePath: "widgets/notes"
    }
    elementId: plasmoid.configuration.color + "-notes"

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    //deliberately not PlasmaComponents.textEdit
    //as we have custom font selection
    PlasmaExtras.ScrollArea {
        id: mainScrollArea
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: units.largeSpacing
            bottom: fontButtons.top
            bottomMargin: Math.round(units.largeSpacing / 2)
        }

        Flickable {
            id: flickable
            contentWidth: mainScrollArea.width
            contentHeight: mainTextArea.height

            // TextEdit doesn't handle scrolling while typing itself
            function ensureVisible(rect) {
                if (contentY >= rect.y) {
                    contentY = rect.y
                } else if (contentY + height < rect.y + rect.height) {
                    contentY = rect.y + rect.height - height
                }
            }

            TextEdit {
                id: mainTextArea
                width: parent.width
                height: Math.max(mainScrollArea.height, paintedHeight)
                text: documentHandler.text
                onCursorRectangleChanged: flickable.ensureVisible(cursorRectangle)
                focus: true
                textFormat: Qt.RichText
                wrapMode: TextEdit.Wrap
                renderType: Qt.NativeRendering
            }
        }
    }

    Notes.DocumentHandler {
        id: documentHandler
        target: mainTextArea
        cursorPosition: mainTextArea.cursorPosition
        selectionStart: mainTextArea.selectionStart
        selectionEnd: mainTextArea.selectionEnd

        text: plasmoid.configuration.noteText
    }
    //plasma has it's own internal timers to not trash the config file when things change
    //may as well let it do it
    Binding {
        target: plasmoid.configuration
        property: "noteText"
        value: mainTextArea.text
    }

    Row {
        id: fontButtons
        spacing: units.smallSpacing
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: units.largeSpacing
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
    }

    function actionTriggered(actionName) {
        if (actionName.indexOf("change_note_color_") == 0){
            plasmoid.configuration.color = actionName.replace("change_note_color_", "");
        }
    }
}
