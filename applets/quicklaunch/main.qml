/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.draganddrop as DragAndDrop
import plasma.applet.org.kde.plasma.quicklaunch

import "layout.js" as LayoutManager

PlasmoidItem {
    id: root

    readonly property int maxSectionCount: Plasmoid.configuration.maxSectionCount
    readonly property bool showLauncherNames : Plasmoid.configuration.showLauncherNames
    readonly property bool enablePopup : Plasmoid.configuration.enablePopup
    readonly property string title : Plasmoid.formFactor == PlasmaCore.Types.Planar ? Plasmoid.configuration.title : ""
    readonly property bool vertical : Plasmoid.formFactor == PlasmaCore.Types.Vertical || (Plasmoid.formFactor == PlasmaCore.Types.Planar && height > width)
    readonly property bool horizontal : Plasmoid.formFactor == PlasmaCore.Types.Horizontal
    property bool dragging : false
    property int internalDragIndex: -1
    property int internalDragOriginalIndex: -1

    Layout.minimumWidth: LayoutManager.minimumWidth()
    Layout.minimumHeight: LayoutManager.minimumHeight()
    Layout.preferredWidth: LayoutManager.preferredWidth()
    Layout.preferredHeight: LayoutManager.preferredHeight()

    preferredRepresentation: fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    Item {
        anchors.fill: parent

        PlasmaComponents3.Label {
            id: titleLabel

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            height: Kirigami.Units.iconSizes.sizeForLabels
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            elide: Text.ElideMiddle
            text: root.title
            textFormat: Text.PlainText
        }

        Item {
            id: launcher

            anchors {
                top: root.title.length ? titleLabel.bottom : parent.top
                left: parent.left
                right: !root.vertical && popupArrow.visible ? popupArrow.left : parent.right
                bottom: root.vertical && popupArrow.visible ? popupArrow.top : parent.bottom
            }

            GridView {
                id: gridView
                anchors.fill: parent
                interactive: false
                flow: root.horizontal ? GridView.FlowTopToBottom : GridView.FlowLeftToRight
                cellWidth: LayoutManager.preferredCellWidth()
                cellHeight: LayoutManager.preferredCellHeight()
                visible: count

                model: UrlModel {
                    id: launcherModelInstance
                }

                delegate: IconItem {
                    logic: logicInstance
                    grid: gridView
                    popupModel: null
                    launcherModel: launcherModelInstance
                    onMoveItemToPopup: (item, url) => gridView.moveItemToPopup(item, url)
                }

                function moveItemToPopup(iconItem, url) {
                    if (!popupArrow.visible) {
                        return;
                    }

                    popup.visible = true;
                    const popupMain = popup.mainItem as Popup
                    popupMain.popupModel.insertUrl(popupMain.popupModel.count, url.toString());
                    popupMain.listView.currentIndex = popupMain.popupModel.count - 1;
                    iconItem.removeLauncher();
                }
            }

            Kirigami.Icon {
                id: defaultIcon
                anchors.fill: parent
                source: "fork"
                visible: !gridView.visible

                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    mainText: i18nc("@info:tooltip widget title", "Quicklaunch")
                    subText: i18nc("@info:tooltip widget subtitle", "Add launchers by Drag and Drop or by using the context menu.")
                    location: Plasmoid.location
                }
            }
        }

        PlasmaCore.Dialog {
            id: popup
            type: PlasmaCore.Dialog.PopupMenu
            flags: Qt.WindowStaysOnTopHint
            hideOnWindowDeactivate: true
            location: Plasmoid.location
            visualParent: root.vertical ? popupArrow : root

            mainItem: Popup {
                Keys.onEscapePressed: popup.visible = false
                launcherModel: launcherModelInstance
                logic: logicInstance
            }
        }

        PlasmaCore.ToolTipArea {
            id: popupArrow
            visible: root.enablePopup
            location: Plasmoid.location

            anchors {
                top: root.vertical ? undefined : parent.top
                right: parent.right
                bottom: parent.bottom
            }

            subText: popup.visible ? i18nc("@info:tooltip for expander arrow", "Hide icons") : i18nc("@info:tooltip for expander arrow", "Show hidden icons")

            MouseArea {
                id: arrowMouseArea
                anchors.fill: parent

                activeFocusOnTab: parent.visible

                Keys.onPressed: event => {
                    switch (event.key) {
                    case Qt.Key_Space:
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Select:
                        arrowMouseArea.clicked(null);
                        break;
                    }
                }
                Accessible.name: parent.subText
                Accessible.role: Accessible.Button

                onClicked: {
                    popup.visible = !popup.visible
                }

                Kirigami.Icon {
                    anchors.fill: parent

                    rotation: popup.visible ? 180 : 0
                    Behavior on rotation {
                        RotationAnimation {
                            duration: Kirigami.Units.shortDuration * 3
                        }
                    }

                    source: {
                        if (Plasmoid.location == PlasmaCore.Types.TopEdge) {
                            return "arrow-down";
                        } else if (Plasmoid.location == PlasmaCore.Types.LeftEdge) {
                            return "arrow-right";
                        } else if (Plasmoid.location == PlasmaCore.Types.RightEdge) {
                            return "arrow-left";
                        } else if (root.vertical) {
                            return "arrow-right";
                        } else {
                            return "arrow-up";
                        }
                    }
                }
            }
        }

        DragAndDrop.DropArea {
            id: dropArea
            anchors.fill: parent
            preventStealing: true
            enabled: !Plasmoid.immutable

            function gridIndexAt(eventX, eventY) {
                const pos = gridView.mapFromItem(dropArea, eventX, eventY);
                return gridView.indexAt(pos.x, pos.y);
            }

            onDragEnter: event => {
                if (event.mimeData.hasUrls) {
                    root.dragging = true;
                    root.internalDragIndex = -1;
                    root.internalDragOriginalIndex = -1;

                    const urls = event.mimeData.urls;
                    if (urls.length === 1) {
                        const dragUrl = urls[0].toString();
                        const modelUrls = launcherModelInstance.urls();
                        for (let i = 0; i < modelUrls.length; ++i) {
                            if (modelUrls[i].toString() === dragUrl) {
                                root.internalDragIndex = i;
                                root.internalDragOriginalIndex = i;
                                break;
                            }
                        }
                    }
                } else {
                    event.ignore();
                }
            }

            onDragMove: event => {
                const index = dropArea.gridIndexAt(event.x, event.y);

                if (root.internalDragIndex >= 0) {
                    if (index >= 0 && index !== root.internalDragIndex) {
                        launcherModelInstance.move(root.internalDragIndex, index, 1);
                        root.internalDragIndex = index;
                    }
                } else {
                    launcherModelInstance.showDropMarker(index);
                }

                popup.visible = root.childAt(event.x, event.y) == popupArrow;
            }

            onDragLeave: {
                root.dragging = false;
                if (root.internalDragIndex >= 0) {
                    if (root.internalDragIndex !== root.internalDragOriginalIndex) {
                        launcherModelInstance.move(root.internalDragIndex, root.internalDragOriginalIndex, 1);
                    }
                    root.internalDragIndex = -1;
                    root.internalDragOriginalIndex = -1;
                } else {
                    launcherModelInstance.clearDropMarker();
                }
            }

            onDrop: event => {
                root.dragging = false;

                if (root.internalDragIndex >= 0) {
                    root.internalDragIndex = -1;
                    root.internalDragOriginalIndex = -1;
                    event.accept(Qt.IgnoreAction);
                    root.saveConfiguration();
                } else {
                    const index = dropArea.gridIndexAt(event.x, event.y);
                    launcherModelInstance.clearDropMarker();
                    const urls = event.mimeData.urls;
                    event.accept(event.proposedAction);
                    Qt.callLater(function() {
                        launcherModelInstance.insertUrls(index == -1 ? launcherModelInstance.count : index, urls);
                    });
                }
            }
        }
    }

    Logic {
        id: logicInstance

        onLauncherAdded: (url, isPopup) => {
            const m = isPopup ? (popup.mainItem as Popup).popupModel : launcherModelInstance;
            m.appendUrl(url);
        }

        onLauncherEdited: (url, index, isPopup) => {
            const m = isPopup ? (popup.mainItem as Popup).popupModel : launcherModelInstance;
            m.changeUrl(index, url);
        }
    }

    // States to fix binding loop with enabled popup
    states: [
        State {
            name: "normal"
            when: !root.vertical

            PropertyChanges {
                popupArrow.width: Kirigami.Units.iconSizes.smallMedium
                popupArrow.height: root.height
            }
        },

        State {
            name: "vertical"
            when: root.vertical

            PropertyChanges {
                popupArrow.width: root.width
                popupArrow.height: Kirigami.Units.iconSizes.smallMedium
            }
        }
    ]

    Connections {
        target: Plasmoid.configuration
        function onLauncherUrlsChanged() {
            if (root.dragging) return;
            const configUrls = Plasmoid.configuration.launcherUrls;
            const modelUrls = launcherModelInstance.urls();
            if (configUrls.length === modelUrls.length) {
                let same = true;
                for (let i = 0; i < configUrls.length; ++i) {
                    if (configUrls[i].toString() !== modelUrls[i].toString()) {
                        same = false;
                        break;
                    }
                }
                if (same) return;
            }
            launcherModelInstance.urlsChanged.disconnect(root.saveConfiguration);
            launcherModelInstance.setUrls(Plasmoid.configuration.launcherUrls);
            launcherModelInstance.urlsChanged.connect(root.saveConfiguration);
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@action", "Add Launcher…")
            icon.name: "list-add"
            onTriggered: logicInstance.addLauncher()
        }
    ]

    Component.onCompleted: {
        launcherModelInstance.setUrls(Plasmoid.configuration.launcherUrls);
        launcherModelInstance.urlsChanged.connect(saveConfiguration);
    }

    function saveConfiguration()
    {
        if (!dragging) {
            Plasmoid.configuration.launcherUrls = launcherModelInstance.urls();
        }
    }
}
