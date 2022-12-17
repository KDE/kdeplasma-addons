/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0 as QtDialogs

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.private.colorpicker 2.0 as ColorPicker
import "logic.js" as Logic

Item {
    id: root

    readonly property bool isVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property color recentColor: historyModel.count > 0 ? historyModel.get(0).color : "#00000000" // transparent as fallback
    readonly property string defaultFormat: plasmoid.configuration.defaultFormat
    readonly property int maxColorCount: 9

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    function addColorToHistory(color) {
        // this .toString() is needed otherwise Qt completely screws it up
        // replacing *all* items in the list by the new items and other nonsense
        historyModel.insert(0, {"color": color.toString()});
        // limit to 9 entries
        if (historyModel.count > maxColorCount) {
            historyModel.remove(maxColorCount);
        }
        historyModel.save();
    }

    function colorPicked(color) {
        if (color != recentColor) {
            addColorToHistory(color)
        }

        if (plasmoid.configuration.autoClipboard) {
            picker.copyToClipboard(Logic.formatColor(color, root.defaultFormat))
        }
    }

    function pickColor() {
        plasmoid.expanded = false
        picker.pick()
    }

    ColorPicker.GrabWidget {
        id: picker
        onCurrentColorChanged: colorPicked(currentColor)
    }

    QtDialogs.ColorDialog {
        id: colorDialog
        title: plasmoid.title
        color: recentColor
        onColorChanged: colorPicked(color)
    }

    // prevents the popup from actually opening, needs to be queued
    Timer {
        id: delayedPickTimer
        interval: 0
        onTriggered: root.pickColor()
    }

    ListModel {
        id: historyModel

        function save() {
            let history = [];
            for (let i = 0; i < count; i++) {
               history.push(get(i).color);
            }
            Plasmoid.configuration.history = history;
        }
    }

    Plasmoid.onActivated: {
        if (plasmoid.configuration.pickOnActivate) {
            delayedPickTimer.start()
        }
    }

    function action_clear() {
        historyModel.clear();
        historyModel.save();
    }

    function action_colordialog() {
        colorDialog.open()
    }

    function action_expand() {
        Plasmoid.expanded = true;
    }

    Component.onCompleted: {
        plasmoid.setAction("colordialog", i18nc("@action", "Open Color Dialog"), "color-management")
        plasmoid.setAction("clear", i18nc("@action", "Clear History"), "edit-clear-history")
        plasmoid.setAction("expand", i18nc("@action", "View History"), "color-management")
        Plasmoid.configuration.history.forEach(item => historyModel.append({"color": item}));
        Logic.copyToClipboardText = i18ndc("plasma_applet_org.kde.plasma.colorpicker", "@title:menu", "Copy to Clipboard"); // i18n is not supported in js library
    }

    Plasmoid.compactRepresentation: CompactRepresentation { }

    Plasmoid.fullRepresentation: GridView {
        id: fullRoot

        readonly property int columns: 3

        Layout.minimumWidth: columns * PlasmaCore.Units.gridUnit * 6
        Layout.minimumHeight: Layout.minimumWidth
        Layout.maximumWidth: Layout.minimumWidth
        Layout.maximumHeight: Layout.minimumHeight

        cellWidth: Math.floor(fullRoot.width / fullRoot.columns)
        cellHeight: cellWidth
        boundsBehavior: Flickable.StopAtBounds

        model: historyModel

        highlight: PlasmaExtras.Highlight {}
        highlightMoveDuration: 0

        Loader {
            width: parent.width - PlasmaCore.Units.largeSpacing * 2
            anchors.centerIn: parent
            visible: active

            active: fullRoot.count === 0 && Plasmoid.expanded
            asynchronous: true

            sourceComponent: PlasmaExtras.PlaceholderMessage {
                id: emptyHint

                opacity: 0
                iconName: "edit-none"
                text: i18nc("@info:usagetip", "No colors")

                helpfulAction: QQC2.Action {
                    icon.name: "color-picker"
                    text: i18nc("@action:button", "Pick Color")
                    onTriggered: root.pickColor()
                }

                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.OutCubic
                    property: "opacity"
                    running: true
                    target: emptyHint
                    to: 1
                }
            }
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if (plasmoid.expanded) {
                    fullRoot.forceActiveFocus()
                }
            }
        }

        Keys.onPressed: {
            if (event.key == Qt.Key_Return || event.key == Qt.Key_Menu) {
                if (fullRoot.currentItem) {
                    fullRoot.currentItem.clicked(null);
                }
            } else if (event.key == Qt.Key_Escape) {
                plasmoid.expanded = false;
            }
        }

        // This item serves as a drag pixmap and is captured when a drag starts
        Rectangle {
            id: dragImageDummy
            border {
                color: PlasmaCore.Theme.textColor
                width: Math.round(PlasmaCore.Units.devicePixelRatio)
            }
            radius: width
            width: PlasmaCore.Units.iconSizes.large
            height: PlasmaCore.Units.iconSizes.large
            visible: false
        }

        delegate: MouseArea {
            id: delegateMouse

            readonly property color currentColor: model.color

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

            Keys.onDeletePressed: remove()
            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Space:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                case Qt.Key_Select:
                    delegateMouse.clicked(null);
                    break;
                }
            }
            Accessible.name: colorLabel.text
            Accessible.role: Accessible.ButtonMenu

            onContainsMouseChanged: {
                if (containsMouse) {
                    fullRoot.currentIndex = index
                } else {
                    fullRoot.currentIndex = -1
                }
            }

            onPressed: {
                // grab pixmap only once
                if (Drag.imageSource.toString() === "") { // cannot just do !Drag.imageSource on QUrl
                    dragImageDummy.color = currentColor;
                    dragImageDummy.grabToImage(function (result) {
                        Drag.imageSource = result.url;
                    });
                }
            }

            onClicked: {
                if (mouse.button === Qt.LeftButton) {
                    picker.copyToClipboard(Logic.formatColor(delegateMouse.currentColor, root.defaultFormat))
                    colorLabel.visible = false;
                    copyIndicatorLabel.visible = true;
                    colorLabelRestoreTimer.start()
                } else {
                    Logic.createContextMenu(delegateMouse, delegateMouse.currentColor, picker, colorLabel, copyIndicatorLabel, colorLabelRestoreTimer).open(0, rect.height);
                }
            }

            function remove() {
                historyModel.remove(index);
                historyModel.save();
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
                    margins: PlasmaCore.Units.smallSpacing
                }

                color: delegateMouse.currentColor

                border {
                    color: PlasmaCore.Theme.textColor
                    width: Math.round(PlasmaCore.Units.devicePixelRatio)
                }

                Rectangle {
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        margins: rect.border.width
                    }
                    height: colorLabel.contentHeight + 2 * PlasmaCore.Units.smallSpacing
                    color: PlasmaCore.Theme.backgroundColor
                    opacity: 0.8

                    PlasmaComponents3.Label {
                        id: colorLabel
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideLeft
                        fontSizeMode: Text.HorizontalFit
                        minimumPointSize: PlasmaCore.Theme.smallestFont.pointSize
                        text: Logic.formatColor(delegateMouse.currentColor, root.defaultFormat)
                    }

                    PlasmaComponents3.Label {
                        id: copyIndicatorLabel
                        visible: false
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideLeft
                        fontSizeMode: Text.HorizontalFit
                        minimumPointSize: PlasmaCore.Theme.smallestFont.pointSize
                        text: i18nc("@info:progress just copied a color to clipboard", "Copied!")
                    }
                }
            }

            Loader {
                active: parent.containsMouse || Kirigami.Settings.tabletMode || Kirigami.Settings.hasTransientTouchInput
                anchors.right: parent.right
                anchors.top: parent.top
                sourceComponent: PlasmaComponents3.Button {
                    text: i18nc("@action:button", "Delete")
                    icon.name: "delete"
                    display: PlasmaComponents3.AbstractButton.IconOnly

                    onClicked: delegateMouse.remove()

                    PlasmaComponents3.ToolTip {
                        text: parent.text
                    }
                }
            }
        }
    }
}
