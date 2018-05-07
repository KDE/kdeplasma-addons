/*
 *  Copyright 2015 David Rosca <nowrep@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.2
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0 as DragAndDrop
import org.kde.plasma.private.quicklaunch 1.0

import "layout.js" as LayoutManager

Item {
    id: root

    readonly property int maxSectionCount: plasmoid.configuration.maxSectionCount
    readonly property bool showLauncherNames : plasmoid.configuration.showLauncherNames
    readonly property bool enablePopup : plasmoid.configuration.enablePopup
    readonly property string title : plasmoid.formFactor == PlasmaCore.Types.Planar ? plasmoid.configuration.title : ""
    readonly property bool vertical : plasmoid.formFactor == PlasmaCore.Types.Vertical || (plasmoid.formFactor == PlasmaCore.Types.Planar && height > width)
    readonly property bool horizontal : plasmoid.formFactor == PlasmaCore.Types.Horizontal
    property bool dragging : false

    Layout.minimumWidth: LayoutManager.minimumWidth()
    Layout.minimumHeight: LayoutManager.minimumHeight()
    Layout.preferredWidth: LayoutManager.preferredWidth()
    Layout.preferredHeight: LayoutManager.preferredHeight()

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    DragAndDrop.DropArea {
        anchors.fill: parent
        preventStealing: true
        enabled: !plasmoid.immutable

        onDragEnter: {
            if (event.mimeData.hasUrls) {
                dragging = true;
            } else {
                event.ignore();
            }
        }

        onDragMove: {
            var index = grid.indexAt(event.x, event.y);

            if (isInternalDrop(event)) {
                launcherModel.moveUrl(event.mimeData.source.itemIndex, index);
            } else {
                launcherModel.showDropMarker(index);
            }

            popup.visible = root.childAt(event.x, event.y) == popupArrow;
        }

        onDragLeave: {
            dragging = false;
            launcherModel.clearDropMarker();
        }

        onDrop: {
            dragging = false;
            launcherModel.clearDropMarker();

            if (isInternalDrop(event)) {
                event.accept(Qt.IgnoreAction);
                saveConfiguration();
            } else {
                var index = grid.indexAt(event.x, event.y);
                launcherModel.insertUrls(index == -1 ? launcherModel.count : index, event.mimeData.urls);
                event.accept(event.proposedAction);
            }
        }
    }

    PlasmaComponents.Label {
        id: titleLabel

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        height: theme.mSize(theme.defaultFont).height
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        elide: Text.ElideMiddle
        text: title
    }

    Item {
        id: launcher

        anchors {
            top: title.length ? titleLabel.bottom : parent.top
            left: parent.left
            right: !vertical && popupArrow.visible ? popupArrow.left : parent.right
            bottom: vertical && popupArrow.visible ? popupArrow.top : parent.bottom
        }

        GridView {
            id: grid
            anchors.fill: parent
            interactive: false
            flow: horizontal ? GridView.FlowTopToBottom : GridView.FlowLeftToRight
            cellWidth: LayoutManager.preferredCellWidth()
            cellHeight: LayoutManager.preferredCellHeight()
            visible: count

            model: UrlModel {
                id: launcherModel
            }

            delegate: IconItem { }
        }

        PlasmaCore.IconItem {
            id: defaultIcon
            anchors.fill: parent
            source: "fork"
            visible: !grid.visible

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                icon: "fork"
                mainText: i18n("Quicklaunch")
                subText: i18nc("@info", "Add launchers by Drag and Drop or by using the context menu.")
            }
        }
    }

    PlasmaCore.Dialog {
        id: popup
        type: PlasmaCore.Dialog.PopupMenu
        flags: Qt.WindowStaysOnTopHint
        hideOnWindowDeactivate: true
        location: plasmoid.location
        visualParent: vertical ? popupArrow : root

        mainItem: Popup { }
    }

    PlasmaCore.ToolTipArea {
        id: popupArrow
        visible: enablePopup

        anchors {
            top: vertical ? undefined : parent.top
            right: parent.right
            bottom: parent.bottom
        }

        subText: popup.visible ? i18n("Hide icons") : i18n("Show hidden icons")

        MouseArea {
            id: arrowMouseArea
            anchors.fill: parent

            onClicked: {
                popup.visible = !popup.visible
            }

            PlasmaCore.Svg {
                id: arrowSvg
                imagePath: "widgets/arrows"
            }

            PlasmaCore.SvgItem {
                id: arrow

                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height)
                height: width

                rotation: popup.visible ? 180 : 0
                Behavior on rotation {
                    RotationAnimation {
                        duration: units.shortDuration * 3
                    }
                }

                svg: arrowSvg
                elementId: {
                    if (plasmoid.location == PlasmaCore.Types.TopEdge) {
                        return "down-arrow";
                    } else if (plasmoid.location == PlasmaCore.Types.LeftEdge) {
                        return "right-arrow";
                    } else if (plasmoid.location == PlasmaCore.Types.RightEdge) {
                        return "left-arrow";
                    } else if (vertical) {
                        return "right-arrow";
                    } else {
                        return "up-arrow";
                    }
                }
            }
        }
    }

    Logic {
        id: logic

        onLauncherAdded: {
            var m = isPopup ? popup.mainItem.popupModel : launcherModel;
            m.appendUrl(url);
        }

        onLauncherEdited: {
            var m = isPopup ? popup.mainItem.popupModel : launcherModel;
            m.changeUrl(index, url);
        }
    }

    // States to fix binding loop with enabled popup
    states: [
        State {
            name: "normal"
            when: !vertical

            PropertyChanges {
                target: popupArrow
                width: units.iconSizes.smallMedium
                height: root.height
            }
        },

        State {
            name: "vertical"
            when: vertical

            PropertyChanges {
                target: popupArrow
                width: root.width
                height: units.iconSizes.smallMedium
            }
        }
    ]

    Connections {
        target: plasmoid.configuration
        onLauncherUrlsChanged: {
            launcherModel.urlsChanged.disconnect(saveConfiguration);
            launcherModel.setUrls(plasmoid.configuration.launcherUrls);
            launcherModel.urlsChanged.connect(saveConfiguration);
        }
    }

    Component.onCompleted: {
        launcherModel.setUrls(plasmoid.configuration.launcherUrls);
        launcherModel.urlsChanged.connect(saveConfiguration);

        plasmoid.setAction("addLauncher", i18nc("@action", "Add Launcher..."), "list-add");
    }

    function action_addLauncher()
    {
        logic.addLauncher();
    }

    function saveConfiguration()
    {
        if (!dragging) {
            plasmoid.configuration.launcherUrls = launcherModel.urls();
        }
    }

    function isInternalDrop(event)
    {
        return event.mimeData.source
            && event.mimeData.source.GridView
            && event.mimeData.source.GridView.view == grid;
    }
}
