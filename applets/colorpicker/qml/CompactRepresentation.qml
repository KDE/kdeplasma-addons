/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid
import plasma.applet.org.kde.plasma.colorpicker as ColorPicker

import "Logic.js" as Logic

DropArea {
    id: dropArea

    required property bool isVertical
    required property HistoryModel historyModel

    signal copyToClipboardRequested(string color)
    signal pickColorRequested()
    signal toggleExpandedRequested

    implicitHeight: grid.implicitHeight
    implicitWidth: grid.implicitWidth

    Layout.fillHeight: !isVertical
    Layout.fillWidth: isVertical

    Layout.minimumWidth: isVertical ? 0 : grid.implicitWidth
    Layout.minimumHeight: isVertical ? grid.implicitHeight : 0

    Layout.maximumWidth: height * (2 + Plasmoid.configuration.compactPreviewCount)
    Layout.maximumHeight: width * (2 + Plasmoid.configuration.compactPreviewCount)

    readonly property real buttonSize: isVertical ? parent.width : parent.height
    property bool containsAcceptableDrag: false


    onEntered: drag => {
        containsAcceptableDrag = (drag.hasColor || drag.hasUrls || utils.isValidColor(drag.text));
    }
    onExited: containsAcceptableDrag = false
    onDropped: drop => {
        if (drop.hasColor) {
            dropArea.historyModel.addColorToHistory(drop.colorData)
        } else if (utils.isValidColor(drop.text)) {
            dropArea.historyModel.addColorToHistory(drop.text)
        } else if (drop.hasUrls) {
            // Show loading indicator above the pick button if there are more than 1 circle
            const indicatorParent = circleRepeater.count === 1 ? circleRepeater.itemAt(0) : pickColorButton;
            Logic.showLoadingIndicator(indicatorParent, drop.urls);
            const component = Qt.createComponent("ImageColors.qml");
            drop.urls.forEach(path => {
                let incubator = component.incubateObject(indicatorParent, {
                    "source": path,
                    "loadingIndicator": indicatorParent.loadingIndicator,
                }, Qt.Asynchronous);
                if (incubator.status == Component.Ready) {
                    (incubator.object as ImageColors).addToHistoryRequested.connect(color => {
                        dropArea.historyModel.addColorToHistory(color);
                    });
                } else {
                    incubator.onStatusChanged = (status) => {
                        if (status == Component.Ready) {
                            (incubator.object as ImageColors).addToHistoryRequested.connect(color => {
                                dropArea.historyModel.addColorToHistory(color);
                            });
                        }
                    }
                }
            });
            component.destroy();
        }
        containsAcceptableDrag = false
    }

    ColorPicker.Utils{
        id: utils
    }

    GridLayout {
        id: grid

        anchors.fill: parent

        columns: dropArea.isVertical ? 1 : (1 + (circleRepeater.count > 0 ? circleRepeater.count + 1 : 0))
        rows: dropArea.isVertical ? (1 + (circleRepeater.count > 0 ? circleRepeater.count + 1 : 0)) : 1
        rowSpacing: 0
        columnSpacing: 0

        PlasmaComponents3.ToolButton {
            id: pickColorButton

            Layout.preferredWidth: dropArea.buttonSize
            Layout.preferredHeight: dropArea.buttonSize
            Layout.alignment: Qt.AlignLeft

            property Item loadingIndicator: null

            display: PlasmaComponents3.AbstractButton.IconOnly
            text: i18nc("@info:tooltip", "Pick color")

            onClicked: dropArea.pickColorRequested()

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                mainText: parent.text
                subText: xi18nc("@info:usagetip", "Drag a color code here to save it<nl/>Drag an image file here to get its average color");
            }

            Kirigami.Icon {
                id: pickerIcon
                anchors.centerIn: parent
                width: Math.round(Math.min(parent.width, parent.height) * 0.9)
                height: width
                source: "color-picker"
                active: parent.hovered
            }
        }

        Item { // spacer
            id: spacer

            readonly property real thickness: dropArea.buttonSize / Kirigami.Units.iconSizes.small

            Layout.preferredWidth: dropArea.isVertical ? parent.width : thickness
            Layout.preferredHeight: dropArea.isVertical ? thickness : parent.height
            Layout.alignment: Qt.AlignHCenter
            visible: circleRepeater.count > 0

            Rectangle {
                anchors.centerIn: parent
                width: circleRepeater.count > 0 ? Math.min(parent.width, (circleRepeater.itemAt(0) as ColorCircle).colorCircle.width) : 0
                height: circleRepeater.count > 0 ? Math.min(parent.height, (circleRepeater.itemAt(0) as ColorCircle).colorCircle.height) : 0
                color: Kirigami.Theme.textColor
                opacity: 0.6
            }
        }

        Repeater {
            id: circleRepeater

            model: Plasmoid.configuration.compactPreviewCount

            delegate: ColorCircle {
                required property int index

                Layout.preferredWidth: dropArea.buttonSize
                Layout.preferredHeight: dropArea.buttonSize

                Layout.maximumWidth: (root.width - pickColorButton.width - spacer.width) / Plasmoid.configuration.compactPreviewCount
                Layout.maximumHeight: Layout.maximumWidth
                Layout.alignment: Qt.AlignRight
                color: dropArea.historyModel.count > index ? dropArea.historyModel.get(index).color : "#00000000" // transparent as fallback
                hasColor: dropArea.historyModel.count > index
                viewBackgroundColor: grid.Kirigami.Theme.backgroundColor
                forceChecked: (dropArea.containsAcceptableDrag && index === 0)

                onCopyToClipboardRequested: color => dropArea.copyToClipboardRequested(color)
                onToggleExpandedRequested: dropArea.toggleExpandedRequested()
            }
        }
    }
}
