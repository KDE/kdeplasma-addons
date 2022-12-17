/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.colorpicker 2.0 as ColorPicker

import "logic.js" as Logic

DropArea {
    id: dropArea

    Layout.minimumWidth: root.isVertical ? PlasmaCore.Units.iconSizes.small : (grid.width + spacer.implicitWidth)
    Layout.minimumHeight: root.isVertical ? (grid.height + spacer.implicitHeight) : PlasmaCore.Units.iconSizes.small
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight

    readonly property real buttonSize: root.isVertical ? parent.width : parent.height
    property bool containsAcceptableDrag: false

    onEntered: containsAcceptableDrag = (drag.hasColor || drag.hasUrls || ColorPicker.Utils.isValidColor(drag.text))
    onExited: containsAcceptableDrag = false
    onDropped: {
        if (drop.hasColor) {
            addColorToHistory(drop.colorData)
        } else if (ColorPicker.Utils.isValidColor(drop.text)) {
            addColorToHistory(drop.text)
        } else if (drop.hasUrls) {
            // Show loading indicator above the pick button if there are more than 1 circle
            const indicatorParent = circleRepeater.count === 1 ? circleRepeater.itemAt(0) : pickColorButton;
            Logic.showLoadingIndicator(indicatorParent, drop.urls);
            const component = Qt.createComponent("ImageColors.qml");
            drop.urls.forEach(path => {
                component.incubateObject(indicatorParent, {
                    "source": path,
                    "loadingIndicator": indicatorParent.loadingIndicator,
                }, Qt.Asynchronous);
            });
            component.destroy();
        }
        containsAcceptableDrag = false
    }

    GridLayout {
        id: grid

        width: root.isVertical ? dropArea.buttonSize : implicitWidth
        height: root.isVertical ? implicitHeight : dropArea.buttonSize

        columns: root.isVertical ? 1 : (1 + (circleRepeater.count > 0 ? circleRepeater.count + 1 : 0))
        rows: root.isVertical ? (1 + (circleRepeater.count > 0 ? circleRepeater.count + 1 : 0)) : 1
        rowSpacing: 0
        columnSpacing: 0

        PlasmaComponents3.ToolButton {
            id: pickColorButton

            Layout.preferredWidth: dropArea.buttonSize
            Layout.preferredHeight: dropArea.buttonSize

            property Item loadingIndicator: null

            display: PlasmaComponents3.AbstractButton.IconOnly
            text: i18nc("@info:tooltip", "Pick color")

            onClicked: root.pickColor()

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                mainText: parent.text
                subText: xi18nc("@info:usagetip", "Drag a color code here to save it<nl/>Drag an image file here to get its average color");
            }

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

            readonly property real thickness: dropArea.buttonSize / PlasmaCore.Units.iconSizes.small

            Layout.preferredWidth: root.isVertical ? parent.width : thickness
            Layout.preferredHeight: root.isVertical ? thickness : parent.height
            visible: circleRepeater.count > 0

            Rectangle {
                anchors.centerIn: parent
                width: circleRepeater.count > 0 ? Math.min(parent.width, circleRepeater.itemAt(0).colorCircle.width) : 0
                height: circleRepeater.count > 0 ? Math.min(parent.height, circleRepeater.itemAt(0).colorCircle.height) : 0
                color: PlasmaCore.Theme.textColor
                opacity: 0.6
            }
        }

        Repeater {
            id: circleRepeater

            model: Plasmoid.configuration.compactPreviewCount

            delegate: ColorCircle {
                Layout.preferredWidth: dropArea.buttonSize
                Layout.preferredHeight: dropArea.buttonSize

                color: historyModel.count > index ? historyModel.get(index).color : "#00000000" // transparent as fallback
            }
        }
    }
}
