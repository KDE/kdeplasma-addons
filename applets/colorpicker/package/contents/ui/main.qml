/*
 *  Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Controls 1.1 as QtControls
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.draganddrop 2.0

import org.kde.plasma.private.colorpicker 2.0 as ColorPicker
import "../code/logic.js" as Logic

Item {
    id: root

    readonly property bool isVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property color recentColor: plasmoid.configuration.history[0] || theme.highlightColor // nice default color
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

    ColorPicker.GrabWidget {
        id: picker
        onCurrentColorChanged: {
            if (currentColor != recentColor) {
                addColorToHistory(currentColor)
            }

            if (plasmoid.configuration.autoClipboard) {
                picker.copyToClipboard(Logic.formatColor(currentColor, root.defaultFormat))
            }
        }
    }

    // prevents the popup from actually opening, needs to be queued
    Timer {
        id: delayedPickTimer
        interval: 0
        onTriggered: {
            plasmoid.expanded = false
            picker.pick()
        }
    }

    Plasmoid.onActivated: {
        if (plasmoid.configuration.pickOnActivate) {
            delayedPickTimer.start()
        }
    }

    function action_clear() {
        plasmoid.configuration.history = []
    }

    Component.onCompleted: {
        plasmoid.setAction("clear", i18n("Clear History"), "edit-clear-history")
    }

    Plasmoid.compactRepresentation: Grid {
        readonly property int buttonSize: root.isVertical ? width : height

        columns: root.isVertical ? 1 : 3
        rows: root.isVertical ? 3 : 1

        Layout.minimumWidth: isVertical ? units.iconSizes.small : ((height * 2) + spacer.width)
        Layout.minimumHeight: isVertical ? ((width * 2) + spacer.height) : units.iconSizes.small

        PlasmaComponents.ToolButton {
            width: buttonSize
            height: buttonSize
            tooltip: i18n("Pick Color")
            onClicked: picker.pick()

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

            readonly property int thickness: Math.ceil(Math.min(parent.width, parent.height) / units.iconSizes.small)

            width: root.isVertical ? parent.width : thickness
            height: root.isVertical ? thickness : parent.height

            Rectangle {
                anchors.centerIn: parent
                width: Math.min(parent.width, colorCircle.height)
                height: Math.min(parent.height, colorCircle.height)
                color: theme.textColor
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
            onDragEnter: containsAcceptableDrag = event.mimeData.hasColor()
            onDragLeave: containsAcceptableDrag = false
            onDrop: {
                if (event.mimeData.hasColor()) {
                    addColorToHistory(event.mimeData.color)
                }
                containsAcceptableDrag = false
            }

            PlasmaComponents.ToolButton {
                anchors.fill: parent
                tooltip: i18n("Color Options")
                onClicked: plasmoid.expanded = !plasmoid.expanded
                // indicate viable drag...
                checked: dropArea.containsAcceptableDrag
                checkable: checked

                Rectangle {
                    id: colorCircle
                    anchors.centerIn: parent
                    // try to match the color-picker icon in size
                    width: units.roundToIconSize(pickerIcon.width) * 0.75
                    height: units.roundToIconSize(pickerIcon.height) * 0.75
                    radius: width / 2
                    color: root.recentColor
                }
            }
        }
    }

    Plasmoid.fullRepresentation: GridView {
        id: fullRoot

        readonly property int columns: 3

        Layout.minimumWidth: columns * units.gridUnit * 6
        Layout.minimumHeight: Layout.minimumWidth
        Layout.maximumWidth: Layout.minimumWidth
        Layout.maximumHeight: Layout.minimumHeight

        cellWidth: Math.floor(fullRoot.width / fullRoot.columns)
        cellHeight: cellWidth
        boundsBehavior: Flickable.StopAtBounds

        model: plasmoid.configuration.history

        highlight: PlasmaComponents.Highlight {}
        highlightMoveDuration: 0

        PlasmaComponents.Button {
            anchors.centerIn: parent
            text: i18n("Pick Color")
            visible: fullRoot.count === 0
            onClicked: picker.pick()
        }

        Connections {
            target: plasmoid
            onExpandedChanged: {
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
                    margins: units.smallSpacing
                }

                color: delegateMouse.currentColor

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: colorLabel.contentHeight + 2 * units.smallSpacing
                    color: theme.backgroundColor
                    opacity: 0.8

                    PlasmaComponents.Label {
                        id: colorLabel
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideLeft
                        fontSizeMode: Text.HorizontalFit
                        minimumPointSize: theme.smallestFont.pointSize
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
