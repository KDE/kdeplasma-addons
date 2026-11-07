/*
 *   SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.Menu {
    id: contextMenu

    /**
     * Always show all actions regardless of visible property
     */
    property bool showAllActions: false

    /**
     * A list of extra actions in the context menu.
     */
    property list<QtObject> actions

    Repeater {
        model: contextMenu.actions
        delegate: QQC2.MenuItem {
            required property Kirigami.Action modelData
            text: modelData.text || modelData.tooltip
            icon.name: modelData.icon.name
            onTriggered: modelData.trigger()
            enabled: modelData.enabled
            visible: modelData.visible || contextMenu.showAllActions

            Accessible.description: modelData.Accessible.description
        }
    }

    onClosed: if (parent) parent.forceActiveFocus()
}
