/*
 *   SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid

import plasma.applet.org.kde.plasma.timer as TimerPlasmoid

Item {
    id: compactRepresentation

    required property bool inPanel
    required property bool isVertical
    required property bool running
    required property bool alertMode
    required property int seconds

    signal toggleExpandedRequested
    signal toggleTimerRequested
    signal adjustTimerRequested(int delta)

    Layout.minimumWidth: inPanel ? grid.width : -1
    Layout.minimumHeight: inPanel ? grid.height : -1

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
            if (compactRepresentation.inPanel) {
                return compactRepresentation.isVertical ? CompactRepresentation.LayoutType.VerticalPanel : CompactRepresentation.LayoutType.HorizontalPanel;
            }
            if (compactRepresentation.parent.width - iconItem.Layout.preferredWidth >= remainingTimeLabel.contentWidth) {
                return CompactRepresentation.LayoutType.HorizontalDesktop;
            }
            if (compactRepresentation.parent.height - iconItem.Layout.preferredHeight >= remainingTimeLabel.contentHeight) {
                return CompactRepresentation.LayoutType.VerticalDesktop;
            }
            return CompactRepresentation.LayoutType.IconOnly;
        }
    }

    Keys.onUpPressed: if (!running) { adjustTime(15); }
    Keys.onDownPressed: if (!running) { adjustTime(-15); }

    function adjustTime(value) {
        const stepsize = Plasmoid.configuration.showSeconds ? 1 : 60
        while (value >= 15 && compactRepresentation.seconds + stepsize < 24*60*60) {
            compactRepresentation.adjustTimerRequested(stepsize)
            value -= 15;
        }
        while (value <= -15) {
            compactRepresentation.adjustTimerRequested(-1 * stepsize)
            value += 15;
        }
        return value;
    }

    WheelHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        enabled: !compactRepresentation.running
        onWheel: {
            event.accepted = true;
            rotation = compactRepresentation.adjustTime(rotation);
        }
    }

    GridLayout {
        id: grid

        width: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.HorizontalPanel:
            case CompactRepresentation.LayoutType.HorizontalDesktop:
                return implicitWidth;
            case CompactRepresentation.LayoutType.VerticalPanel:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return compactRepresentation.parent.width;
            case CompactRepresentation.LayoutType.IconOnly:
                return iconItem.Layout.preferredWidth;
            }
        }
        height: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.HorizontalPanel:
            case CompactRepresentation.LayoutType.HorizontalDesktop:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return compactRepresentation.parent.height;
            case CompactRepresentation.LayoutType.VerticalPanel:
                return implicitHeight;
            case CompactRepresentation.LayoutType.IconOnly:
                return iconItem.Layout.preferredHeight;
            }
        }

        rowSpacing: 0
        columnSpacing: rowSpacing
        flow: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.VerticalPanel:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return GridLayout.TopToBottom;
            default:
                return GridLayout.LeftToRight;
            }
        }

        Item {
            id: spacerItem
            Layout.fillHeight: true
            visible: compactRepresentation.layoutForm === CompactRepresentation.LayoutType.VerticalDesktop
        }

        PlasmaComponents3.ToolButton {
            id: iconItem

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Math.min(compactRepresentation.parent.width, compactRepresentation.parent.height)
            Layout.preferredHeight: Layout.preferredWidth
            visible: Plasmoid.configuration.showTimerToggle

            display: PlasmaComponents3.AbstractButton.IconOnly
            icon.name: {
                if (compactRepresentation.running) {
                    return "chronometer-pause";
                }
                return compactRepresentation.seconds > 0 ? "chronometer-start" : "chronometer";
            }
            text: compactRepresentation.running ? i18nc("@action:button", "Pause Timer") : i18nc("@action:button", "Start Timer")

            onClicked: {
                if (compactRepresentation.seconds === 0) {
                    compactRepresentation.toggleExpandedRequested()
                } else {
                    compactRepresentation.toggleTimerRequested()
                }
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.fillWidth: compactRepresentation.layoutForm === CompactRepresentation.LayoutType.VerticalPanel || compactRepresentation.layoutForm === CompactRepresentation.LayoutType.VerticalDesktop
            Layout.maximumWidth: {
                switch (compactRepresentation.layoutForm) {
                case CompactRepresentation.LayoutType.HorizontalPanel:
                    return Kirigami.Units.gridUnit * 10;
                case CompactRepresentation.LayoutType.HorizontalDesktop:
                    return compactRepresentation.parent.width - iconItem.Layout.preferredWidth;
                default:
                    return -1;
                }
            }
            Layout.maximumHeight: textMetrics.height * 2
            visible: compactRepresentation.layoutForm !== CompactRepresentation.LayoutType.IconOnly ? 1 : 0

            spacing: parent.columnSpacing

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: compactRepresentation.toggleExpandedRequested()
            }

            PlasmaComponents3.Label {
                id: titleLabel

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: Math.max(implicitWidth, Plasmoid.configuration.showRemainingTime && remainingTimeLabel.implicitWidth)

                visible: Plasmoid.configuration.showTitle && text !== ""

                elide: Text.ElideRight
                font.bold: remainingTimeLabel.font.bold
                fontSizeMode: remainingTimeLabel.fontSizeMode
                horizontalAlignment: remainingTimeLabel.horizontalAlignment
                verticalAlignment: Text.AlignVCenter
                minimumPointSize: remainingTimeLabel.minimumPointSize
                text: Plasmoid.configuration.title
                textFormat: Text.PlainText
            }

            PlasmaComponents3.Label {
                id: remainingTimeLabel

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: Math.max(Plasmoid.configuration.showTitle && titleLabel.implicitWidth, implicitWidth)
                visible: Plasmoid.configuration.showRemainingTime

                TextMetrics {
                    id: textMetrics
                    // make it not jump around: reserve space for one extra digit than reasonable
                    text: Plasmoid.configuration.showSeconds ? "44:44:444" : "44:444";
                    font: remainingTimeLabel.font
                }

                activeFocusOnTab: true
                elide: compactRepresentation.inPanel ? Text.ElideRight : Text.ElideNone
                font.bold: compactRepresentation.alertMode
                font.features: { "tnum": 1 }
                fontSizeMode: compactRepresentation.layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || compactRepresentation.layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? Text.VerticalFit : Text.HorizontalFit
                horizontalAlignment: compactRepresentation.layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || compactRepresentation.layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? Text.AlignJustify : Text.AlignHCenter
                minimumPointSize: 1
                verticalAlignment: Text.AlignVCenter

                text: (Plasmoid.configuration.showSeconds || !Plasmoid.configuration.showRemainingTime)
                    ? TimerPlasmoid.Timer.secondsToString(compactRepresentation.seconds, "hh:mm:ss")
                    : TimerPlasmoid.Timer.secondsToString(compactRepresentation.seconds, "hh:mm");
                textFormat: Text.PlainText

                Accessible.name: root.toolTipMainText
                Accessible.description: root.toolTipSubText
                Accessible.role: Accessible.Button
            }

            PlasmaComponents3.ProgressBar {
                id: remainingTimeProgressBar

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: compactRepresentation.inPanel
                     ? Math.max(Plasmoid.configuration.showTitle && titleLabel.implicitWidth, remainingTimeLabel.implicitWidth)
                     : -1
                visible: Plasmoid.configuration.showProgressBar

                from: Plasmoid.configuration.seconds
                to: 0
                value: compactRepresentation.seconds
            }
        }

        Item {
            Layout.fillHeight: true
            visible: spacerItem.visible
        }
    }
}
