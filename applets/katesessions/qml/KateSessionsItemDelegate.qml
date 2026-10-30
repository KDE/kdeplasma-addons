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
import org.kde.kirigami as Kirigami

PlasmaComponents3.ItemDelegate {
    id: menuItem

    required property string name
    required property string profileIdentifier
    required property string iconName
    required property int index

    signal itemSelected(string profileIdentifier)

    property bool showInput: false

    height: Math.max(iconTitle.implicitHeight, sessionname.implicitHeight, okButton.implicitHeight) + Kirigami.Units.smallSpacing
    Binding {
        when: menuItem.showInput
        menuItem.topPadding: Math.round(Kirigami.Units.smallSpacing / 2)
        menuItem.bottomPadding: Math.round(Kirigami.Units.smallSpacing / 2)
    }

    onClicked: {
        if (profileIdentifier !== "")
            menuItem.itemSelected(profileIdentifier);
        else {
            showInput=true;
            sessionname.forceActiveFocus(Qt.MouseFocusReason);
        }
    }

    contentItem: RowLayout {
        Kirigami.IconTitleSubtitle {
            id: iconTitle
            Layout.fillWidth: !menuItem.showInput
            title: menuItem.showInput ? "" : menuItem.name.trim()
            icon.name: menuItem.iconName
            icon.width: Kirigami.Units.iconSizes.small
            icon.height: Kirigami.Units.iconSizes.small
        }

        PlasmaComponents3.TextField {
            id: sessionname
            visible: menuItem.showInput
            placeholderText: i18n("Session name")
            clearButtonShown: true
            Layout.fillWidth: true
            Keys.onReturnPressed: okButton.clicked()
            Keys.onEscapePressed: cancelButton.clicked()
        }

        PlasmaComponents3.ToolButton {
            id: okButton
            visible: menuItem.showInput
            enabled: sessionname.text.replace(/^\s+|\s+$/g, '').length>0
            text: i18n("Create new session and start Kate")
            icon.name: "dialog-ok"
            display: PlasmaComponents3.ToolButton.IconOnly
            PlasmaComponents3.ToolTip.visible: hovered || visualFocus
            PlasmaComponents3.ToolTip.text: text
            PlasmaComponents3.ToolTip.delay: Kirigami.Units.toolTipDelay
            onClicked: {
                menuItem.itemSelected(sessionname.text.replace(/^\s+|\s+$/g, ''));
                menuItem.showInput=false
            }
        }

        PlasmaComponents3.ToolButton {
            id: cancelButton
            visible: menuItem.showInput
            text: i18n("Cancel session creation")
            icon.name: "dialog-cancel"
            display: PlasmaComponents3.ToolButton.IconOnly
            PlasmaComponents3.ToolTip.visible: hovered || visualFocus
            PlasmaComponents3.ToolTip.text: text
            PlasmaComponents3.ToolTip.delay: Kirigami.Units.toolTipDelay
            onClicked: {
                menuItem.showInput=false;
                sessionname.text='';
            }
        }
    }
}
