/*
    SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    Based on the clipboard applet:
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.profiles

PlasmaComponents3.ItemDelegate {
    id: menuItem

    required property string name
    required property string profileIdentifier
    required property string iconName
    required property int index

    signal itemSelected(string profileIdentifier)

    property bool showInput: false

    height: Math.max(label.height, sessionnameditlayout.implicitHeight) + Kirigami.Units.smallSpacing

    onClicked: {
        if (profileIdentifier !== "")
            menuItem.itemSelected(profileIdentifier);
        else {
            showInput=true;
            sessionname.forceActiveFocus(Qt.MouseFocusReason);
        }
    }

    Item {
        id: label
        height: iconItem.height
        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.smallSpacing
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        PlasmaComponents3.Label {
            anchors {
                left: parent.left
                right: parent.right
                rightMargin: Kirigami.Units.gridUnit * 2
                leftMargin: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
                verticalCenter: parent.verticalCenter
            }
            maximumLineCount: 1
            text: name.trim()
            textFormat: Text.PlainText
            visible: !menuItem.showInput
            elide: Text.ElideRight
            wrapMode: Text.Wrap
        }

        Kirigami.Icon {
            id: iconItem
            width: Kirigami.Units.iconSizes.small
            height: width
            anchors.verticalCenter: parent.verticalCenter
            source: iconName
        }
    }

    RowLayout {
        id:sessionnameditlayout
        visible:menuItem.showInput
        height: implicitHeight
        anchors {
            left: parent.left
            right: parent.right
            rightMargin: 0
            leftMargin: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
            verticalCenter: parent.verticalCenter
        }

        PlasmaComponents3.TextField {
            id: sessionname
            placeholderText: i18n("Session name")
            clearButtonShown: true
            Layout.fillWidth: true
            Keys.onReturnPressed: {menuItem.itemSelected(sessionname.text.replace(/^\s+|\s+$/g, '')); showInput=false;}
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
            onClicked: {
                menuItem.showInput=false;
                sessionname.text='';
            }

            PlasmaComponents3.ToolTip {
                text: i18n("Cancel session creation")
            }
        }
    }
}

