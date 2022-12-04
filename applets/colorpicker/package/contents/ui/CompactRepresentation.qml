/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.private.colorpicker 2.0 as ColorPicker

DropArea {
    id: dropArea

    Layout.minimumWidth: root.isVertical ? PlasmaCore.Units.iconSizes.small : (grid.implicitHeight * 2 + spacer.width)
    Layout.minimumHeight: root.isVertical ? (grid.implicitWidth * 2 + spacer.height) : PlasmaCore.Units.iconSizes.small

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
            colorCircle.showLoadingIndicator(drop.urls);
            const component = Qt.createComponent("ImageColors.qml");
            drop.urls.forEach(path => {
                component.incubateObject(colorCircle, {
                    "source": path,
                    "loadingIndicator": colorCircle.loadingIndicator,
                }, Qt.Asynchronous);
            });
            component.destroy();
        }
        containsAcceptableDrag = false
    }

    Grid {
        id: grid

        anchors.fill: parent

        columns: root.isVertical ? 1 : 3
        rows: root.isVertical ? 3 : 1
        rowSpacing: 0
        columnSpacing: 0

        PlasmaComponents3.ToolButton {
            width: dropArea.buttonSize
            height: dropArea.buttonSize

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

            width: root.isVertical ? parent.width : thickness
            height: root.isVertical ? thickness : parent.height

            Rectangle {
                anchors.centerIn: parent
                width: Math.min(parent.width, colorCircle.colorCircle.width)
                height: Math.min(parent.height, colorCircle.colorCircle.height)
                color: PlasmaCore.Theme.textColor
                opacity: 0.6
            }
        }

        ColorCircle {
            id: colorCircle

            width: dropArea.buttonSize
            height: dropArea.buttonSize

            color: root.recentColor
        }
    }
}
