/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwin 2.0 as KWin

KWin.Switcher {
    id: tabBox
    currentIndex: listView.currentIndex

    PlasmaCore.Dialog {
        id: informativeTabBox
        property bool canStretchX: true
        property bool canStretchY: false

        property int textMargin: 2

        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        mainItem: Item {
            id: dialogMainItem
            property int optimalWidth: listView.maxRowWidth
            property int optimalHeight: listView.rowHeight * listView.count
            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.2, optimalWidth), tabBox.screenGeometry.width * 0.8)
            height: Math.min(Math.max(tabBox.screenGeometry.height * 0.2, optimalHeight), tabBox.screenGeometry.height * 0.8)
            focus: true

            // just to get the margin sizes
            PlasmaCore.FrameSvgItem {
                id: hoverItem
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: false
            }

            // delegate
            Component {
                id: listDelegate
                Item {
                    id: delegateItem
                    width: listView.width
                    height: listView.rowHeight
                    QIconItem {
                        id: iconItem
                        icon: model.icon
                        width: PlasmaCore.Units.iconSizes.medium
                        height: PlasmaCore.Units.iconSizes.medium
                        state: index == listView.currentIndex ? QIconItem.ActiveState : QIconItem.DisabledState
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: hoverItem.margins.left
                        }
                    }
                    PlasmaComponents3.Label {
                        id: captionItem
                        horizontalAlignment: Text.AlignHCenter
                        text: listView.itemCaption(caption, minimized)
                        font.weight: Font.Bold
                        font.italic: minimized
                        elide: Text.ElideMiddle
                        anchors {
                            left: iconItem.right
                            right: parent.right
                            top: parent.top
                            topMargin: informativeTabBox.textMargin + hoverItem.margins.top
                            rightMargin: hoverItem.margins.right
                        }
                    }
                    PlasmaComponents3.Label {
                        id: desktopNameItem
                        horizontalAlignment: Text.AlignHCenter
                        text: desktopName
                        font.italic: true
                        elide: Text.ElideMiddle
                        visible: tabBox.allDesktops
                        anchors {
                            left: iconItem.right
                            right: parent.right
                            top: captionItem.bottom
                            topMargin: informativeTabBox.textMargin
                            bottom: parent.bottom
                            bottomMargin: informativeTabBox.textMargin + hoverItem.margins.bottom
                            rightMargin: hoverItem.margins.right
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            listView.currentIndex = index;
                        }
                    }
                }
            }
            ListView {
                /**
                * Returns the caption with adjustments for minimized items.
                * @param caption the original caption
                * @param mimized whether the item is minimized
                * @return Caption adjusted for minimized state
                **/
                function itemCaption(caption, minimized) {
                    var text = caption;
                    if (minimized) {
                        text = "(" + text + ")";
                    }
                    return text;
                }
                function calculateMaxRowWidth() {
                    var width = 0;
                    var textElement = Qt.createQmlObject(
                        'import QtQuick 2.0;'
                        + 'Text {\n'
                        + '     text: "' + listView.itemCaption(tabBox.model.longestCaption(), true) + '"\n'
                        + '     font.bold: true\n'
                        + '     visible: false\n'
                        + '}',
                        listView, "calculateMaxRowWidth");
                    width = Math.max(textElement.width, width);
                    textElement.destroy();
                    return width + PlasmaCore.Units.iconSizes.medium + hoverItem.margins.right + hoverItem.margins.left;
                }
                /**
                * Calculates the height of one row based on the text height and icon size.
                * @return Row height
                **/
                function calcRowHeight() {
                    var textElement = Qt.createQmlObject(
                        'import QtQuick 2.0;'
                        + 'Text {\n'
                        + '     text: "Some Text"\n'
                        + '     font.bold: true\n'
                        + '     visible: false\n'
                        + '}',
                        listView, "calcRowHeight");
                    var height = textElement.height;
                    textElement.destroy();
                    // icon size or two text elements and margins and hoverItem margins
                    return Math.max(PlasmaCore.Units.iconSizes.medium, height*2 + informativeTabBox.textMargin * 3 + hoverItem.margins.top + hoverItem.margins.bottom);
                }
                id: listView
                model: tabBox.model
                // the maximum text width + icon item width (32 + 4 margin) + margins for hover item + margins for background
                property int maxRowWidth: calculateMaxRowWidth()
                property int rowHeight: calcRowHeight()
                anchors {
                    fill: parent
                }
                clip: true
                delegate: listDelegate
                highlight: PlasmaCore.FrameSvgItem {
                    id: highlightItem
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                    width: listView.width
                }
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                boundsBehavior: Flickable.StopAtBounds
                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {listView.currentIndex = tabBox.currentIndex;}
                }
            }

            /*
            * Key navigation on outer item for two reasons:
            * @li we have to emit the change signal
            * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
            **/
            Keys.onPressed: {
                if (event.key == Qt.Key_Up) {
                    listView.decrementCurrentIndex();
                } else if (event.key == Qt.Key_Down) {
                    listView.incrementCurrentIndex();
                }
            }
        }
    }
}
