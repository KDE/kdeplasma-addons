/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kwin as KWin

KWin.TabBoxSwitcher {
    id: tabBox

    currentIndex: (instantiator.object as BigIconsDialog)?.currentIndex ?? currentIndex

    Instantiator {
        id: instantiator
        active: tabBox.visible
        delegate: BigIconsDialog { }
    }

    component BigIconsDialog: PlasmaCore.Dialog {
        property alias currentIndex: icons.currentIndex
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.Popup | Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        mainItem: ColumnLayout {
            id: dialogMainItem
            spacing: Kirigami.Units.smallSpacing * 2

            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.3, icons.implicitWidth), tabBox.screenGeometry.width * 0.9)

            property int maxItemsPerRow:  Math.floor(tabBox.screenGeometry.width * 0.9 / icons.delegateWidth)
            property int actualItemsPerRow:  Math.min(tabBox.model.rowCount(), maxItemsPerRow)

            property int gridViewWidth: Math.max(4, actualItemsPerRow) * icons.delegateWidth
            property int gridViewHeight: Math.ceil(tabBox.model.rowCount() / maxItemsPerRow) * icons.delegateHeight

            GridView {
                id: icons

                readonly property int iconSize: Kirigami.Units.iconSizes.enormous
                readonly property int delegateWidth: iconSize + (highlightItem ? highlightItem.margins.left + highlightItem.margins.right : 0)
                readonly property int delegateHeight: iconSize + (highlightItem ? highlightItem.margins.top + highlightItem.margins.bottom : 0)

                implicitWidth: dialogMainItem.actualItemsPerRow * delegateWidth
                implicitHeight: dialogMainItem.gridViewHeight
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: tabBox.screenGeometry.width * 0.9
                Layout.fillWidth: false // to make centering with few icons work
                Layout.fillHeight: true
                cellWidth: delegateWidth
                cellHeight: delegateHeight

                currentIndex: tabBox.currentIndex
                focus: true
                flow: GridView.LeftToRight
                keyNavigationWraps: true

                model: tabBox.model
                delegate: Kirigami.Icon {
                    property string caption: model.caption

                    width: icons.delegateWidth
                    height: icons.delegateHeight

                    source: model.icon
                    active: index == icons.currentIndex

                    Accessible.name: caption

                    TapHandler {
                        onSingleTapped: {
                            if (index === icons.currentIndex) {
                                icons.model.activate(index);
                                return;
                            }
                            icons.currentIndex = index;
                        }
                        onDoubleTapped: icons.model.activate(index)
                    }
                }

                highlight: KSvg.FrameSvgItem {
                    id: highlightItem
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                    width: icons.iconSize + margins.left + margins.right
                    height: icons.iconSize + margins.top + margins.bottom
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: Math.round(captionLabel.height / 2)
                    width: parent.width - Kirigami.Units.largeSpacing * 2
                    icon.source: "edit-none"
                    text: i18ndc("kwin", "@info:placeholder no entries in the task switcher", "No open windows")
                    visible: icons.count === 0
                }

                highlightMoveDuration: 0
                boundsBehavior: Flickable.StopAtBounds
            }

            PlasmaComponents3.Label {
                id: captionLabel
                text: icons.currentItem ? icons.currentItem.caption : ""
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
                font.weight: Font.Bold
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
            }

            Connections {
                target: tabBox
                function onCurrentIndexChanged() {
                    icons.currentIndex = tabBox.currentIndex;
                }
            }

            /*
             * Key navigation on outer item for two reasons:
             * @li we have to emit the change signal
             * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
             **/
            Keys.onPressed: event => {
                if (event.key === Qt.Key_Up) {
                    icons.moveCurrentIndexUp()
                } else if (event.key === Qt.Key_Down) {
                    icons.moveCurrentIndexDown()
                } else if (event.key === Qt.Key_Left) {
                    icons.moveCurrentIndexLeft()
                } else if (event.key === Qt.Key_Right) {
                    icons.moveCurrentIndexRight()
                }
            }
        }

        onSceneGraphError: () => {
            // This slot is intentionally left blank, otherwise QtQuick may post a qFatal() message on a graphics reset.
        }
    }
}
