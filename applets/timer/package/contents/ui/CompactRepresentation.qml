/*
 *   SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.timer 0.1 as TimerPlasmoid

Item {
    id: compactRepresentation

    Layout.minimumHeight: root.inPanel ? Layout.preferredHeight : -1

    Layout.preferredWidth: {
        switch (layoutForm) {
        case CompactRepresentation.LayoutType.HorizontalPanel:
        case CompactRepresentation.LayoutType.HorizontalDesktop:
            return iconItem.width + column.width;
        case CompactRepresentation.LayoutType.VerticalPanel:
        case CompactRepresentation.LayoutType.VerticalDesktop:
            return parent.width;
        case CompactRepresentation.LayoutType.IconOnly:
            return -1;
        }
    }
    Layout.preferredHeight: {
        switch (layoutForm) {
        case CompactRepresentation.LayoutType.HorizontalPanel:
        case CompactRepresentation.LayoutType.HorizontalDesktop:
            return parent.height;
        case CompactRepresentation.LayoutType.VerticalPanel:
        case CompactRepresentation.LayoutType.VerticalDesktop:
            return iconItem.height + column.height;
        case CompactRepresentation.LayoutType.IconOnly:
            return -1;
        }
    }

    enum LayoutType {
        HorizontalPanel,
        VerticalPanel,
        HorizontalDesktop,
        VerticalDesktop,
        IconOnly
    }

    property int layoutForm

    Binding on layoutForm {
        delayed: true
        value: {
            if (root.inPanel) {
                return root.isVertical ? CompactRepresentation.LayoutType.VerticalPanel : CompactRepresentation.LayoutType.HorizontalPanel;
            }
            if (compactRepresentation.parent.width - Math.min(parent.width, parent.height) >= remainingTimeLabel.contentWidth) {
                return CompactRepresentation.LayoutType.HorizontalDesktop;
            }
            if (compactRepresentation.parent.height - Math.min(parent.width, parent.height) >= remainingTimeLabel.contentHeight) {
                return CompactRepresentation.LayoutType.VerticalDesktop;
            }
            return CompactRepresentation.LayoutType.IconOnly;
        }
    }

    Keys.onUpPressed: adjustSecond(10);
    Keys.onDownPressed: adjustSecond(-10);

    function adjustSecond(value) {
        if (value > 5) {
            if (root.seconds + 1 < 24*60*60) {
                root.seconds += 1;
            }
        } else if (value < -5) {
            if (root.seconds - 1 >= 0) {
                root.seconds -= 1;
            }
        }
    }

    WheelHandler {
        enabled: !root.running
        onWheel: {
            event.accepted = true;
            compactRepresentation.adjustSecond(event.angleDelta.y);
        }
    }

    PlasmaComponents3.ToolButton {
        id: iconItem

        anchors.left: parent.left
        anchors.horizontalCenter: compactRepresentation.layoutForm >= 3 ? parent.horizontalCenter : undefined
        width: Math.min(compactRepresentation.parent.width, compactRepresentation.parent.height)
        height: width

        display: PlasmaComponents3.AbstractButton.IconOnly
        icon.name: {
            if (root.running) {
                return "chronometer-pause";
            }
            return root.seconds > 0 ? "chronometer-start" : "chronometer";
        }
        text: root.running ? i18nc("@action:button", "Pause Timer") : i18nc("@action:button", "Start Timer")

        onClicked: {
            if (root.seconds === 0) {
                Plasmoid.expanded = !Plasmoid.expanded;
            } else {
                root.toggleTimer();
            }
        }
    }

    ColumnLayout {
        id: column

        anchors.left: layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? iconItem.right : parent.left
        anchors.top: layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? parent.top : iconItem.bottom
        width: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.HorizontalPanel:
                return Math.min(implicitWidth, PlasmaCore.Units.gridUnit * 10);
            case CompactRepresentation.LayoutType.HorizontalDesktop:
                return Math.min(implicitWidth, compactRepresentation.parent.width - iconItem.width);
            case CompactRepresentation.LayoutType.VerticalPanel:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return compactRepresentation.parent.width;
            case CompactRepresentation.LayoutType.IconOnly:
                return 0;
            }
        }
        height: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.HorizontalPanel:
            case CompactRepresentation.LayoutType.HorizontalDesktop:
                return compactRepresentation.parent.height;
            case CompactRepresentation.LayoutType.VerticalPanel:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return implicitHeight;
            case CompactRepresentation.LayoutType.IconOnly:
                return 0;
            }
        }

        // Use opacity not visible to make contentHeight work
        opacity: compactRepresentation.layoutForm !== CompactRepresentation.LayoutType.IconOnly ? 1 : 0
        spacing: 0

        PlasmaComponents3.Label {
            id: titleLabel

            Layout.fillWidth: true
            visible: root.showTitle && root.title !== ""

            elide: Text.ElideRight
            font.bold: remainingTimeLabel.font.bold
            fontSizeMode: remainingTimeLabel.fontSizeMode
            horizontalAlignment: remainingTimeLabel.horizontalAlignment
            minimumPointSize: remainingTimeLabel.minimumPointSize
            text: root.title
        }

        PlasmaComponents3.Label {
            id: remainingTimeLabel

            Layout.alignment: layoutForm == CompactRepresentation.LayoutType.VerticalPanel || layoutForm === CompactRepresentation.LayoutType.VerticalDesktop ? Qt.AlignHCenter : Qt.AlignLeft
            Layout.fillWidth: false
            Layout.maximumWidth: textMetrics.width
            Layout.minimumWidth: textMetrics.width

            TextMetrics {
                id: textMetrics
                text: {
                    if (root.isVertical) {
                        return i18ncp("remaining time", "%1s", "%1s", root.seconds);
                    }
                    // make it not jump around: reserve space for one extra digit than reasonable
                    return root.showSeconds ? "44:44:444" : "44:444";
                }
                font: remainingTimeLabel.font
            }

            activeFocusOnTab: true
            elide: root.inPanel ? Text.ElideRight : Text.ElideNone
            font.bold: root.alertMode
            fontSizeMode: layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? Text.VerticalFit : Text.HorizontalFit
            horizontalAlignment: layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? Text.AlignJustify : Text.AlignHCenter
            minimumPointSize: PlasmaCore.Theme.smallestFont.pointSize

            text: {
                if (root.isVertical) {
                    return i18ncp("remaining time", "%1s", "%1s", root.seconds);
                }

                return root.showSeconds ? TimerPlasmoid.Timer.secondsToString(root.seconds, "hh:mm:ss") : TimerPlasmoid.Timer.secondsToString(root.seconds, "hh:mm");
            }

            Accessible.name: Plasmoid.toolTipMainText
            Accessible.description: Plasmoid.toolTipSubText
            Accessible.role: Accessible.Button
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: Plasmoid.expanded = !Plasmoid.expanded
        }
    }
}
