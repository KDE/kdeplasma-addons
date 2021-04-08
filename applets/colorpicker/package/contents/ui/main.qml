/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Controls 1.1 as QtControls
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0 as QtDialogs

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents // for Highlight and ModelContextMenu and deficiencies with PC3 ToolButton+ToolTip (see inline TODOs)
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.draganddrop 2.0

import org.kde.plasma.private.colorpicker 2.0 as ColorPicker
import "logic.js" as Logic

Item {
    id: root

    readonly property bool isVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property color recentColor: plasmoid.configuration.history[0] || "#00000000" // transparent as fallback
    readonly property string defaultFormat: plasmoid.configuration.defaultFormat

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.toolTipSubText: Logic.formatColor(recentColor, defaultFormat)

    function addColorToHistory(color) {
        // this is needed, otherwise the first pick after plasma start isn't registered
        var history = plasmoid.configuration.history

        // this .toString() is needed otherwise Qt completely screws it up
        // replacing *all* items in the list by the new items and other nonsense
        history.unshift(color.toString())

        // limit to 9 entries
        plasmoid.configuration.history = history.slice(0, 9)
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

    Plasmoid.onActivated: {
        if (plasmoid.configuration.pickOnActivate) {
            delayedPickTimer.start()
        }
    }

    function action_clear() {
        plasmoid.configuration.history = []
    }

    function action_colordialog() {
        colorDialog.open()
    }

    Component.onCompleted: {
        plasmoid.setAction("colordialog", i18nc("@action", "Open Color Dialog"), "color-management")
        plasmoid.setAction("clear", i18nc("@action", "Clear History"), "edit-clear-history")
    }

    Plasmoid.compactRepresentation: Grid {
        readonly property int buttonSize: root.isVertical ? width : height

        columns: root.isVertical ? 1 : 3
        rows: root.isVertical ? 3 : 1

        Layout.minimumWidth: isVertical ? PlasmaCore.Units.iconSizes.small : ((height * 2) + spacer.width)
        Layout.minimumHeight: isVertical ? ((width * 2) + spacer.height) : PlasmaCore.Units.iconSizes.small

        // TODO: Still PC2 for now because:
        // - It never loses visible focus despite panel widgets never getting focus anyway: https://bugs.kde.org/show_bug.cgi?id=424446
        // - PC3.ToolTip has visual glitches: https://bugs.kde.org/show_bug.cgi?id=424448
        PlasmaComponents.ToolButton {
            width: buttonSize
            height: buttonSize
            tooltip: i18nc("@info:tooltip", "Pick color")
            onClicked: root.pickColor()

            PlasmaCore.IconItem {
                id: pickerIcon
                anchors.centerIn: parent
                width: Math.round(parent.width * 0.9)
                height: width
                source: "color-picker"
                active: parent.hovered
            }
        }

        Item { // spacer
            id: spacer

            readonly property int thickness: Math.ceil(Math.min(parent.width, parent.height) / PlasmaCore.Units.iconSizes.small)

            width: root.isVertical ? parent.width : thickness
            height: root.isVertical ? thickness : parent.height

            Rectangle {
                anchors.centerIn: parent
                width: Math.min(parent.width, colorCircle.height)
                height: Math.min(parent.height, colorCircle.height)
                color: PlasmaCore.Theme.textColor
                opacity: 0.6
            }
        }

        // this is so the circle we put ontop of the ToolButton resizes
        // exactly like the regular icon on the other button
        PlasmaCore.FrameSvgItem {
            id: surfaceNormal
            imagePath: "widgets/button"
            prefix: "normal"
            visible: false
        }

        DropArea {
            id: dropArea

            property bool containsAcceptableDrag: false

            width: buttonSize
            height: buttonSize
            preventStealing: true
            // why the hell is hasColor not a property?!
            onDragEnter: containsAcceptableDrag = (event.mimeData.hasColor() || ColorPicker.Utils.isValidColor(event.mimeData.text))
            onDragLeave: containsAcceptableDrag = false
            onDrop: {
                if (event.mimeData.hasColor()) {
                    addColorToHistory(event.mimeData.color)
                } else if (ColorPicker.Utils.isValidColor(event.mimeData.text)) {
                    addColorToHistory(event.mimeData.text)
                }
                containsAcceptableDrag = false
            }

            // TODO: Still PC2 for now because:
            // - It never loses visible focus despite panel widgets never getting focus anyway: https://bugs.kde.org/show_bug.cgi?id=424446
            // - PC3.ToolTip visual glitches: https://bugs.kde.org/show_bug.cgi?id=424448
            PlasmaComponents.ToolButton {
                id: colorButton
                anchors.fill: parent
                tooltip: i18nc("@info:tooltip", "Color options")
                onClicked: plasmoid.expanded = !plasmoid.expanded
                // indicate viable drag...
                checked: dropArea.containsAcceptableDrag || colorButtonDragger.pressed
                checkable: checked

                Rectangle {
                    id: colorCircle
                    anchors.centerIn: parent
                    // try to match the color-picker icon in size
                    width: PlasmaCore.Units.roundToIconSize(pickerIcon.width) * 0.75
                    height: PlasmaCore.Units.roundToIconSize(pickerIcon.height) * 0.75
                    radius: width / 2
                    color: root.recentColor

                    function luminance(color) {
                        if (!color) {
                            return 0;
                        }

                        // formula for luminance according to https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef

                        var a = [color.r, color.g, color.b].map(function (v) {
                            return (v <= 0.03928) ? v / 12.92 :
                                                    Math.pow( ((v + 0.055) / 1.055), 2.4 );
                        });

                        return a[0] * 0.2126 + a[1] * 0.7152 + a[2] * 0.0722;
                    }

                    border {
                        color: PlasmaCore.Theme.textColor
                        width: {
                            var contrast = luminance(PlasmaCore.Theme.viewBackgroundColor) / luminance(colorCircle.color) + 0.05;

                            // show border only if there's too little contrast to the surrounding view or color is transparent
                            if (contrast > 3 && colorCircle.color.a > 0.5) {
                                return 0;
                            } else {
                                return Math.round(Math.max(PlasmaCore.Units.devicePixelRatio, width / 20));
                            }
                        }
                    }
                }

                MouseArea {
                    id: colorButtonDragger
                    anchors.fill: parent
                    onClicked: colorButton.clicked()

                    onPressed: colorCircle.grabToImage(function (result) {
                        Drag.imageSource = result.url;
                    });

                    drag.target: colorCircle
                    Drag.dragType: Drag.Automatic
                    Drag.active: drag.active
                    Drag.mimeData: {
                        "application/x-color": colorCircle.color,
                        "text/plain": Logic.formatColor(colorCircle.color, root.defaultFormat)
                    }
                }
            }
        }
    }

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

        model: plasmoid.configuration.history

        highlight: PlasmaComponents.Highlight {}
        highlightMoveDuration: 0

        PlasmaComponents3.Button {
            anchors.centerIn: parent
            text: i18nc("@action:button", "Pick Color")
            visible: fullRoot.count === 0
            onClicked: root.pickColor()
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if (plasmoid.expanded) {
                    fullRoot.forceActiveFocus()
                }
            }
        }

        QtControls.Action {
            shortcut: "Return"
            onTriggered: {
                if (fullRoot.currentItem) {
                    fullRoot.currentItem.clicked(null)
                }
            }
        }

        QtControls.Action {
            shortcut: "Escape"
            onTriggered: plasmoid.expanded = false
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

            readonly property color currentColor: modelData

            width: fullRoot.cellWidth
            height: fullRoot.cellHeight

            drag.target: rect
            Drag.dragType: Drag.Automatic
            Drag.active: delegateMouse.drag.active
            Drag.mimeData: {
                "application/x-color": rect.color,
                "text/plain": colorLabel.text
            }

            hoverEnabled: true
            onContainsMouseChanged: {
                if (containsMouse) {
                    fullRoot.currentIndex = index
                } else {
                    fullRoot.currentIndex = -1
                }
            }

            onPressed: {
                // grab pixmap only once
                if (Drag.imageSource.toString() === "") { // cannot just do !Drage.imageSource on QUrl
                    dragImageDummy.color = currentColor;
                    dragImageDummy.grabToImage(function (result) {
                        Drag.imageSource = result.url;
                    });
                }
            }

            onClicked: {
                formattingMenu.model = Logic.menuForColor(delegateMouse.currentColor)
                formattingMenu.open(0, rect.height)
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                active: colorLabel.truncated
                mainText: colorLabel.text
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
                }

                PlasmaComponents.ModelContextMenu {
                    id: formattingMenu
                    onClicked: picker.copyToClipboard(model.text)
                }
            }
        }
    }
}
