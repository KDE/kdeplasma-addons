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
    required property real maxWidth

    signal goToIndex(int index)

    readonly property int pageCount: pageSize > 0 ? Math.ceil(count / pageSize) : 0

    readonly property int currentPage: pageSize > 0 ? Math.floor(currentIndex / pageSize) : 0

    Layout.preferredHeight: Math.max(goFirstButton.implicitHeight, goPreviousButton.implicitHeight, goNextButton.implicitHeight, goLastButton.implicitHeight, indicator.implicitHeight)

    spacing: Kirigami.Units.smallSpacing

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

        readonly property int largePointSize: Kirigami.Units.largeSpacing

        readonly property real freeWidth: Math.max(0, root.maxWidth - goFirstButton.width - goPreviousButton.width - goNextButton.width - goLastButton.width - root.spacing * 4 - padding * 2 - largePointSize)

        readonly property int maxPoints: Math.max(1, Math.floor((freeWidth + spacing) / (largePointSize + spacing)))

        readonly property int visibleCount: Math.min(indicator.count, indicator.maxPoints)

        readonly property int firstVisibleIndex: {
            if (count <= maxPoints) {
                return 0;
            }

            const half = Math.floor(maxPoints / 2);

            return Math.min(Math.max(0, currentIndex - half), count - maxPoints);
        }

        Layout.alignment: Qt.AlignVCenter

        delegate: Rectangle {
            required property int index

            readonly property int delegateIndex: indicator.firstVisibleIndex + index
            readonly property bool hasPrevious: indicator.firstVisibleIndex > 0
            readonly property bool hasNext: indicator.firstVisibleIndex + indicator.visibleCount < indicator.count

            readonly property bool isLeadingIndicator: index === 0 && hasPrevious
            readonly property bool isTrailingIndicator: index === indicator.visibleCount - 1 && hasNext

            implicitWidth: Kirigami.Units.largeSpacing
            implicitHeight: implicitWidth

            radius: width
            color: delegateIndex === indicator.currentIndex ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
            opacity: delegateIndex === indicator.currentIndex ? 1.0 : 0.4

            scale: isLeadingIndicator || isTrailingIndicator ? 0.7 : 1

            TapHandler {
                onTapped: root.goToIndex(delegateIndex * root.pageSize)
            }
        }

        contentItem: Row {
            spacing: indicator.spacing

            Repeater {
                model: indicator.visibleCount
                delegate: indicator.delegate
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
