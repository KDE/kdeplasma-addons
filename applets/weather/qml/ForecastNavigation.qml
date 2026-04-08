/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

RowLayout {
    id: root

    required property int currentIndex
    required property int pageSize
    required property int count

    signal goToIndex(int index)

    readonly property int pageCount: pageSize > 0 ? Math.ceil(count / pageSize) : 0

    readonly property int currentPage: pageSize > 0 ? Math.floor(currentIndex / pageSize) : 0

    Layout.alignment: Qt.AlignHCenter

    Layout.preferredHeight: Math.max(goFirstButton.implicitHeight, goPreviousButton.implicitHeight, goNextButton.implicitHeight, goLastButton.implicitHeight, indicator.implicitHeight)

    spacing: 0

    PlasmaComponents.ToolButton {
        id: goFirstButton

        icon.name: mirrored ? "go-first-rtl-symbolic" : "go-first-symbolic"
        text: i18nc("@action", "Go to first day")
        display: PlasmaComponents.AbstractButton.IconOnly

        PlasmaComponents.ToolTip.text: text
        PlasmaComponents.ToolTip.visible: hovered || activeFocus
        PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay

        enabled: root.currentIndex > 0
        visible: indicator.visible

        onClicked: root.goToIndex(0)

        Layout.preferredWidth: Kirigami.Units.iconSizes.small
    }

    PlasmaComponents.ToolButton {
        id: goPreviousButton

        icon.name: mirrored ? "go-previous-rtl-symbolic" : "go-previous-symbolic"
        text: i18nc("@action", "Go to previous day")
        display: PlasmaComponents.AbstractButton.IconOnly

        PlasmaComponents.ToolTip.text: text
        PlasmaComponents.ToolTip.visible: hovered || activeFocus
        PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay

        enabled: root.currentIndex > 0
        visible: indicator.visible

        onClicked: root.goToIndex(Math.max(0, root.currentIndex - root.pageSize))

        Layout.preferredWidth: Kirigami.Units.iconSizes.small
    }

    PlasmaComponents.PageIndicator {
        id: indicator

        count: root.pageCount
        currentIndex: root.currentPage

        Layout.alignment: Qt.AlignHCenter

        visible: count > 1

        delegate: Rectangle {
            width: Math.round(Kirigami.Units.gridUnit * 0.5)
            height: width
            radius: Math.round(width / 2)

            color: index === indicator.currentIndex ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

            opacity: index === indicator.currentIndex ? 1.0 : 0.4

            MouseArea {
                anchors.fill: parent

                onClicked: root.goToIndex(index * root.pageSize)
            }
        }
    }

    PlasmaComponents.ToolButton {
        id: goNextButton

        icon.name: mirrored ? "go-next-rtl-symbolic" : "go-next-symbolic"
        text: i18nc("@action", "Go to next day")
        display: PlasmaComponents.AbstractButton.IconOnly

        PlasmaComponents.ToolTip.text: text
        PlasmaComponents.ToolTip.visible: hovered || activeFocus
        PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay

        visible: indicator.visible
        enabled: root.currentIndex + root.pageSize < root.count

        onClicked: root.goToIndex(Math.min(root.count - 1, root.currentIndex + root.pageSize))

        Layout.preferredWidth: Kirigami.Units.iconSizes.small
    }

    PlasmaComponents.ToolButton {
        id: goLastButton

        icon.name: mirrored ? "go-last-rtl-symbolic" : "go-last-symbolic"
        text: i18nc("@action", "Go to last day")
        display: PlasmaComponents.AbstractButton.IconOnly

        PlasmaComponents.ToolTip.text: text
        PlasmaComponents.ToolTip.visible: hovered || activeFocus
        PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay

        visible: indicator.visible
        enabled: root.currentIndex + root.pageSize < root.count

        onClicked: root.goToIndex(Math.min(root.count - 1, (root.pageCount - 1) * root.pageSize))

        Layout.preferredWidth: Kirigami.Units.iconSizes.small
    }
}
