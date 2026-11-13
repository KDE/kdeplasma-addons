/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs as QtDialogs

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import plasma.applet.org.kde.plasma.colorpicker as ColorPicker
import "Logic.js" as Logic

PlasmoidItem {
    id: root

    readonly property bool isVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property color recentColor: historyModelItem.hasAnyColors ? historyModelItem.get(0).color : "#00000000" // transparent as fallback
    readonly property string defaultFormat: Plasmoid.configuration.defaultFormat

    preferredRepresentation: compactRepresentation

    toolTipMainText: "" // set to empty to prevent automatic tooltip generation through compactRepresentation
    toolTipSubText: ""

    function colorPicked(color) {
        if (color != recentColor) {
            historyModelItem.addColorToHistory(color)
        }

        if (Plasmoid.configuration.autoClipboard) {
            picker.copyToClipboard(Logic.formatColor(color, root.defaultFormat))
        }
    }

    function pickColor() {
        root.expanded = false
        picker.pick()
    }

    ColorPicker.GrabWidget {
        id: picker
        onCurrentColorChanged: root.colorPicked(currentColor)
    }

    Loader {
        id: colorDialogLoader
        active: false
        sourceComponent: QtDialogs.ColorDialog {
            id: colorDialog
            modality: Qt.NonModal
            title: Plasmoid.title
            selectedColor: historyModelItem.count > 0 ? root.recentColor : undefined /* Prevent transparent colors */
            onAccepted: {
                root.colorPicked(selectedColor);
                colorDialogLoader.active = false;
            }
            onRejected: colorDialogLoader.active = false
        }
        onLoaded: (item as QtDialogs.ColorDialog).open()
    }

    // prevents the popup from actually opening, needs to be queued
    Timer {
        id: delayedPickTimer
        interval: 0
        onTriggered: root.pickColor()
    }

    ColorPicker.HistoryModel { id: historyModelItem }

    Plasmoid.onActivated: {
        if (Plasmoid.configuration.pickOnActivate) {
            delayedPickTimer.start();
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@action", "Open Color Dialog")
            icon.name: "color-management"
            onTriggered: colorDialogLoader.active = true
        },
        PlasmaCore.Action {
            text: i18nc("@action", "Clear History")
            icon.name: "edit-clear-history"
            onTriggered: {
                historyModelItem.clear();
                historyModelItem.save();
            }
        },
        PlasmaCore.Action {
            text: i18nc("@action", "View History")
            icon.name: "view-history"
            onTriggered: root.expanded = true
        }
    ]

    Component.onCompleted: {
        Plasmoid.configuration.history.forEach(item => historyModelItem.append({ color: item }));
        Logic.copyToClipboardText = i18ndc("plasma_applet_org.kde.plasma.colorpicker", "@title:menu", "Copy to Clipboard"); // i18n is not supported in js library
    }

    compactRepresentation: CompactRepresentation {
        isVertical: root.isVertical
        historyModel: historyModelItem
        onCopyToClipboardRequested: color=> picker.copyToClipboard(color)
        onPickColorRequested: root.pickColor()
        onToggleExpandedRequested: root.expanded = !root.expanded
    }

    fullRepresentation: GridView {
        id: fullRoot

        readonly property int columns: 3

        Layout.minimumWidth: columns * Kirigami.Units.gridUnit * 6
        Layout.minimumHeight: Layout.minimumWidth
        Layout.maximumWidth: Layout.minimumWidth
        Layout.maximumHeight: Layout.minimumHeight

        cellWidth: Math.floor(fullRoot.width / fullRoot.columns)
        cellHeight: cellWidth
        boundsBehavior: Flickable.StopAtBounds

        model: historyModelItem

        highlight: PlasmaExtras.Highlight {}
        highlightMoveDuration: 0

        Loader {
            width: parent.width - Kirigami.Units.gridUnit * 2
            anchors.centerIn: parent
            visible: active

            active: fullRoot.count === 0 && root.expanded
            asynchronous: true

            sourceComponent: PlasmaExtras.PlaceholderMessage {
                id: emptyHint

                opacity: 0
                iconName: "edit-none"

                text: i18nc("@info:usagetip", "No colors")

                helpfulAction: pickColorAction

                QQC2.Action {
                    id: pickColorAction
                    icon.name: "color-picker"
                    text: i18nc("@action:button", "Pick Color")
                    onTriggered: root.pickColor()
                }

                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutCubic
                    property: "opacity"
                    running: true
                    target: emptyHint
                    to: 1
                }
            }
        }

        Connections {
            target: root
            function onExpandedChanged() {
                if (root.expanded) {
                    fullRoot.forceActiveFocus()
                }
            }
        }

        Keys.onPressed: event => {
            if (event.key === Qt.Key_Escape) {
                root.expanded = false;
                event.accepted = true;
            }
        }

        // This item serves as a drag pixmap and is captured when a drag starts
        Rectangle {
            id: dragImageDummy
            border {
                color: Kirigami.Theme.textColor
                width: 1
            }
            radius: width
            width: Kirigami.Units.iconSizes.large
            height: Kirigami.Units.iconSizes.large
            visible: false
        }

        delegate: MouseArea {
            id: delegateMouse

            required property color color
            required property int index

            width: fullRoot.cellWidth
            height: fullRoot.cellHeight

            drag.target: rect
            Drag.dragType: Drag.Automatic
            Drag.active: delegateMouse.drag.active
            Drag.mimeData: {
                "application/x-color": rect.color,
                "text/plain": colorLabel.text
            }

            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true

            Keys.onDeletePressed: event => remove()
            Keys.onPressed: event => {
                switch (event.key) {
                case Qt.Key_Space:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                case Qt.Key_Select:
                    copy();
                    break;
                case Qt.Key_Menu:
                    openMenu();
                    break;
                }
            }

            Accessible.name: colorLabel.text
            Accessible.role: Accessible.ButtonMenu

            onContainsMouseChanged: {
                if (containsMouse) {
                    fullRoot.currentIndex = index;
                } else if (fullRoot.currentIndex === index) {
                    fullRoot.currentIndex = -1;
                }
            }

            onPressed: mouse => {
                // grab pixmap only once
                if (Drag.imageSource.toString() === "") { // cannot just do !Drag.imageSource on QUrl
                    dragImageDummy.color = delegateMouse.color;
                    dragImageDummy.grabToImage(result => {
                        Drag.imageSource = result.url;
                    });
                }
            }

            onClicked: mouse => {
                if (mouse.button === Qt.LeftButton) {
                    copy();
                } else if (mouse.button === Qt.RightButton) {
                    openMenu();
                }
            }

            function copy() {
                picker.copyToClipboard(Logic.formatColor(delegateMouse.color, root.defaultFormat))
                colorLabel.visible = false;
                copyIndicatorLabel.visible = true;
                colorLabelRestoreTimer.start()
            }

            function openMenu() {
                const menu = Logic.createContextMenu(this, delegateMouse.color, picker, colorLabel, copyIndicatorLabel, colorLabelRestoreTimer) as ColorPicker.ColorContextMenu;
                menu.openRelative();
            }

            function remove() {
                historyModelItem.remove(index);
                historyModelItem.save();
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                active: colorLabel.truncated
                mainText: colorLabel.text
            }

            Timer {
                id: colorLabelRestoreTimer
                interval: Kirigami.Units.humanMoment
                onTriggered: {
                    colorLabel.visible = true;
                    copyIndicatorLabel.visible = false;
                }
            }

            Rectangle {
                id: rect

                anchors {
                    fill: parent
                    margins: Kirigami.Units.smallSpacing
                }

                color: delegateMouse.color

                border {
                    color: Kirigami.Theme.textColor
                    width: 1
                }

                Rectangle {
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        margins: rect.border.width
                    }
                    height: colorLabel.contentHeight + 2 * Kirigami.Units.smallSpacing
                    color: Kirigami.Theme.backgroundColor
                    opacity: 0.8

                    PlasmaComponents3.Label {
                        id: colorLabel
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideLeft
                        fontSizeMode: Text.HorizontalFit
                        minimumPointSize: Kirigami.Theme.smallFont.pointSize
                        text: Logic.formatColor(delegateMouse.color, root.defaultFormat)
                        textFormat: Text.PlainText
                    }

                    PlasmaComponents3.Label {
                        id: copyIndicatorLabel
                        visible: false
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideLeft
                        fontSizeMode: Text.HorizontalFit
                        minimumPointSize: Kirigami.Theme.smallFont.pointSize
                        text: i18nc("@info:progress just copied a color to clipboard", "Copied!")
                        textFormat: Text.PlainText
                    }
                }
            }

            Loader {
                active: parent.containsMouse || Kirigami.Settings.tabletMode || Kirigami.Settings.hasTransientTouchInput
                anchors.right: parent.right
                anchors.top: parent.top
                sourceComponent: PlasmaComponents3.Button {
                    id: deleteButton
                    text: i18nc("@action:button", "Delete")
                    icon.name: "delete"
                    display: PlasmaComponents3.AbstractButton.IconOnly

                    onClicked: delegateMouse.remove()

                    PlasmaComponents3.ToolTip.visible: hovered || visualFocus
                    PlasmaComponents3.ToolTip.text: text
                    PlasmaComponents3.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }
    }
}
