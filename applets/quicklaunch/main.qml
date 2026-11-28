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
import org.kde.ksvg as KSvg
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
                    id: crDelegate
                    logic: logicInstance
                    grid: gridView
                    popupModel: null
                    leftPadding: Math.round(Kirigami.Units.smallSpacing / 2)
                    topPadding: Math.round(Kirigami.Units.smallSpacing / 2)
                    bottomPadding: Math.round(Kirigami.Units.smallSpacing / 2)
                    rightPadding: Math.round(Kirigami.Units.smallSpacing / 2)
                    launcherModel: launcherModelInstance
                    onMoveItemToPopup: (item, url) => gridView.moveItemToPopup(item, url)
                    contentItem: ColumnLayout {
                        spacing: 0
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            source: crDelegate.icon.name
                            active: crDelegate.hovered
                            implicitWidth: Kirigami.Units.iconSizes.medium
                            implicitHeight: Kirigami.Units.iconSizes.medium
                        }
                        PlasmaComponents3.Label {
                            Layout.fillWidth: true
                            visible: Plasmoid.configuration.showLauncherNames
                            text: crDelegate.text
                            maximumLineCount: 1
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.Wrap
                            elide: Text.ElideNone
                        }
                    }
                    background: KSvg.FrameSvgItem {
                        anchors.fill: parent
                        imagePath: "widgets/viewitem"
                        prefix: "hover"
                        visible: crDelegate.Drag.active || crDelegate.url === "quicklaunch:drop"
                    }
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

        PlasmaCore.AppletPopup {
            id: popup
            hideOnWindowDeactivate: true
            visualParent: root.vertical ? popupArrow : root
            popupDirection: switch (Plasmoid.location) {
                case PlasmaCore.Types.TopEdge:
                    return Qt.BottomEdge
                case PlasmaCore.Types.LeftEdge:
                    return Qt.RightEdge
                case PlasmaCore.Types.RightEdge:
                    return Qt.LeftEdge
                default:
                    return Qt.TopEdge
            }
            margin: (Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentPrefersFloatingApplets) ? Kirigami.Units.largeSpacing : 0

            Behavior on margin {
                NumberAnimation {
                    // Since the panel animation won't be perfectly in sync,
                    // using a duration larger than the panel animation results
                    // in a better-looking animation.
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
            }

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

            activeFocusOnTab: parent.visible

            Keys.onSpacePressed: popup.visible = !popup.visible
            Keys.onEnterPressed: event => Keys.spacePressed(event)
            Keys.onReturnPressed: event => Keys.spacePressed(event)
            Keys.onSelectPressed: event => Keys.spacePressed(event)

            Accessible.name: subText
            Accessible.role: Accessible.Button

            TapHandler {
                id: tapHandler
                onTapped: popup.visible = !popup.visible
            }

            Kirigami.Icon {
                anchors.fill: parent

                rotation: popup.visible ? 180 : 0
                Behavior on rotation {
                    RotationAnimation {
                        duration: Kirigami.Units.shortDuration
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

        QuickLaunchDropArea {
            id: dropArea
            anchors.fill: parent
            enabled: !Plasmoid.immutable
            model: gridView.model
            view: gridView

            onSaveConfigurationRequested: root.saveConfiguration();

            onPositionChanged: event => {
                const point = root.mapToItem(popupArrow, event.x, event.y)
                popup.visible = point.x >= 0 && point.x <= popupArrow.width && point.y >= 0 && point.y <= popupArrow.height;
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
            if (dropArea.containsDrag) return;
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

    function saveConfiguration() {
        if (!dropArea.containsDrag) {
            Plasmoid.configuration.launcherUrls = launcherModelInstance.urls();
        }
    }
}
