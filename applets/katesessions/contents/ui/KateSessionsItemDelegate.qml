/*
    SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    Based on the clipboard applet:
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.profiles 1.0

PlasmaComponents.ListItem {
    id: menuItem

    signal itemSelected(string profileIdentifier)
    
    property bool showInput: false

    height: Math.max(label.height, sessionnameditlayout.implicitHeight) + PlasmaCore.Units.smallSpacing

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (profileIdentifier !== "")
                menuItem.itemSelected(profileIdentifier);
            else {
                showInput=true; 
            }
        }
        onEntered: menuListView.currentIndex = index
        onExited: menuListView.currentIndex = -1

        Item {
            id: label
            height: iconItem.height
            anchors {
                left: parent.left
                leftMargin: PlasmaCore.Units.smallSpacing
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            PlasmaComponents3.Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    rightMargin: PlasmaCore.Units.gridUnit * 2
                    leftMargin: PlasmaCore.Units.iconSizes.small + PlasmaCore.Units.smallSpacing * 2
                    verticalCenter: parent.verticalCenter
                }
                maximumLineCount: 1
                text: name.trim()
                visible: !showInput
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }

            PlasmaCore.IconItem {
                id: iconItem
                width: PlasmaCore.Units.iconSizes.small
                height: width
                anchors.verticalCenter: parent.verticalCenter
                source: iconName
            }
        }

        RowLayout {
                id:sessionnameditlayout
                visible:showInput
                height: implicitHeight
                anchors {
                    left: parent.left
                    right: parent.right
                    rightMargin: 0
                    leftMargin: PlasmaCore.Units.iconSizes.small + PlasmaCore.Units.smallSpacing * 2
                    verticalCenter: parent.verticalCenter
                }

                PlasmaComponents3.TextField {
                    id: sessionname
                    placeholderText: i18n("Session name")
                    clearButtonShown: true
                    Layout.fillWidth: true
                }

                PlasmaComponents3.ToolButton {
                    icon.name: "dialog-ok"
                    enabled: sessionname.text.replace(/^\s+|\s+$/g, '').length>0
                    onClicked: {menuItem.itemSelected(sessionname.text.replace(/^\s+|\s+$/g, '')); showInput=false;}

                    PlasmaComponents3.ToolTip {
                        text: i18n("Create new session and start Kate")
                    }
                }

                PlasmaComponents3.ToolButton {
                    icon.name: "dialog-cancel"
                    onClicked: showInput=false

                    PlasmaComponents3.ToolTip {
                        text: i18n("Cancel session creation")
                    }
                }
        }
    }
}
