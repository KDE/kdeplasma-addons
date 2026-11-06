/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrolsaddons
import org.kde.plasma.plasmoid

PlasmaCore.ToolTipArea {
    id: root
    objectName: "org.kde.desktop-CompactApplet"
    anchors.fill: parent

    icon: plasmoid.icon ?? ""
    mainText: plasmoid.toolTipMainText ?? ""
    subText: plasmoid.toolTipSubText ?? ""
    location: plasmoid.location ?? PlasmaCore.Types.Floating
    active: !plasmoidItem.expanded ?? false
    textFormat: plasmoid.toolTipTextFormat ?? Text.AutoText
    mainItem: plasmoid.toolTipItem ? plasmoid.toolTipItem : null
    property PlasmoidItem plasmoidItem

    property Item fullRepresentation
    property Item compactRepresentation

    onCompactRepresentationChanged: {
        if (compactRepresentation) {
            compactRepresentation.parent = root;
            compactRepresentation.anchors.fill = root;
            compactRepresentation.visible = true;
        }
        root.visible = true;
    }

    onFullRepresentationChanged: {

        if (!fullRepresentation) {
            return;
        }
        //if the fullRepresentation size was restored to a stored size, or if is dragged from the desktop, restore popup size
        if (fullRepresentation.width > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.width
            })
        } else if (fullRepresentation.Layout && fullRepresentation.Layout.preferredWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.Layout.preferredWidth
            })
        } else if (fullRepresentation.implicitWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.implicitWidth
            })
        } else {
            popupWindow.mainItem.width = Qt.binding(function() {
                return Kirigami.Units.gridUnit * 35
            })
        }

        if (fullRepresentation.height > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.height
            })
        } else if (fullRepresentation.Layout && fullRepresentation.Layout.preferredHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.Layout.preferredHeight
            })
        } else if (fullRepresentation.implicitHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.implicitHeight
            })
        } else {
            popupWindow.mainItem.height = Qt.binding(function() {
                return Kirigami.Units.gridUnit * 25
            })
        }

        fullRepresentation.parent = appletParent;
        fullRepresentation.anchors.fill = fullRepresentation.parent;
    }

    Timer {
        id: expandedSync
        interval: 100
        onTriggered: root.plasmoidItem.expanded = popupWindow.visible;
    }

    Connections {
        target: plasmoid.internalAction("configure")
        function onTriggered() {
            root.plasmoidItem.expanded = false
        }
    }

    Connections {
        target: plasmoid
        function onContextualActionsAboutToShow() {
            root.hideToolTip()
        }
    }

    PlasmaCore.Dialog {
        id: popupWindow
        objectName: "popupWindow"
        flags: Qt.WindowStaysOnTopHint
        visible: root.plasmoidItem.expanded && root.fullRepresentation
        visualParent: root.compactRepresentation ? root.compactRepresentation : null
        location: plasmoid.location
        hideOnWindowDeactivate: root.plasmoidItem.hideOnWindowDeactivate

        property var oldStatus: PlasmaCore.Types.UnknownStatus

        //It's a MouseEventListener to get all the events, so the eventfilter will be able to catch them
        mainItem: MouseEventListener {
            id: appletParent

            focus: true

            Keys.onEscapePressed: {
                root.plasmoidItem.expanded = false;
            }

            LayoutMirroring.enabled: Application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            Layout.minimumWidth: (root.fullRepresentation && root.fullRepresentation.Layout) ? root.fullRepresentation.Layout.minimumWidth : 0
            Layout.minimumHeight: (root.fullRepresentation && root.fullRepresentation.Layout) ? root.fullRepresentation.Layout.minimumHeight: 0
            Layout.maximumWidth: (root.fullRepresentation && root.fullRepresentation.Layout) ? root.fullRepresentation.Layout.maximumWidth : Infinity
            Layout.maximumHeight: (root.fullRepresentation && root.fullRepresentation.Layout) ? root.fullRepresentation.Layout.maximumHeight: Infinity

            onActiveFocusChanged: {
                if (activeFocus && root.fullRepresentation) {
                    root.fullRepresentation.forceActiveFocus()
                }
            }
        }

        onVisibleChanged: {
            if (!visible) {
                expandedSync.restart();
                plasmoid.status = oldStatus;
            } else {
                oldStatus = plasmoid.status;
                plasmoid.status = PlasmaCore.Types.RequiresAttentionStatus;
                // This call currently fails and complains at runtime:
                // QWindow::setWindowState: QWindow::setWindowState does not accept Qt::WindowActive
                popupWindow.requestActivate();
            }
        }
    }
}
