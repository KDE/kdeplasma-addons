/*
 *   SPDX-FileCopyrightText: 2008, 2014 Davide Bettio <davide.bettio@kdemail.net>
 *   SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras

ColumnLayout {
    id: fullRepresentation

    required property bool isPopup
    required property bool isVertical
    required property bool running
    required property bool suspended
    required property bool alertMode
    required property int seconds

    signal toggleTimerRequested
    signal adjustTimerRequested(int delta)

    Layout.preferredWidth: isPopup ? Math.max(root.compactRepresentationItem?.width ?? 0, Kirigami.Units.gridUnit * 20) : -1
    Layout.preferredHeight: isPopup ? Math.round(Layout.preferredWidth * 2/5) : -1
    Layout.minimumHeight: isPopup ? Layout.preferredHeight : 0

    TapHandler {
        onTapped: fullRepresentation.toggleTimerRequested()
    }

    Component {
        id: popupHeadingComponent

        PlasmaExtras.PlasmoidHeading {
            anchors.fill: parent
            leftPadding: Kirigami.Units.smallSpacing * 2
            rightPadding: Kirigami.Units.smallSpacing * 2

            contentItem: Kirigami.Heading {
                level: 3
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                text: Plasmoid.configuration.title
                textFormat: Text.PlainText
            }
        }
    }

    Component {
        id: desktopHeadingComponent

        PlasmaComponents3.Label {
            anchors.fill: parent
            elide: Text.ElideRight
            fontSizeMode: Text.VerticalFit
            minimumPixelSize: 8
            font.pixelSize: Math.max(height * 0.7, 1)
            text: Plasmoid.configuration.title
            textFormat: Text.PlainText
            horizontalAlignment: PlasmaComponents3.Label.AlignHCenter
        }
    }

    Item {
        id: headerWrapper
        Layout.minimumHeight: fullRepresentation.isPopup ? (headerLoader.item as Item)?.implicitHeight ?? 0 : -1
        Layout.maximumHeight: fullRepresentation.isPopup ? (headerLoader.item as Item)?.implicitHeight ?? 0 : Kirigami.Units.gridUnit * 4
        Layout.fillHeight: fullRepresentation.isPopup ? false : true
        Layout.fillWidth: true
        implicitHeight: 1
        visible: headerLoader.active

        Loader {
            id: headerLoader
            anchors.fill: parent

            active: Plasmoid.configuration.showTitle

            sourceComponent: fullRepresentation.isPopup ? popupHeadingComponent : desktopHeadingComponent
        }
    }

    Item {
        id: timeWrapper
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: Plasmoid.configuration.showRemainingTime || fullRepresentation.isPopup
        implicitHeight: 3

        TimerEdit {
            id: timerDigits
            anchors.centerIn: parent

            value: fullRepresentation.seconds
            editable: !fullRepresentation.running
            alertMode: fullRepresentation.alertMode
            showSeconds: Plasmoid.configuration.showSeconds
            onDigitModified: valueDelta => fullRepresentation.adjustTimerRequested(valueDelta)
            onToggleTimerRequested: fullRepresentation.toggleTimerRequested()
            maximumHeight: timeWrapper.height
            maximumWidth: timeWrapper.width
            SequentialAnimation on opacity {
                running: fullRepresentation.suspended;
                loops: Animation.Infinite;
                onRunningChanged: if (!running) { timerDigits.opacity = 1.0 }
                NumberAnimation {
                    duration: Kirigami.Units.veryLongDuration * 2;
                    from: 1.0;
                    to: 0.2;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: Kirigami.Units.veryLongDuration;
                }
                NumberAnimation {
                    duration: Kirigami.Units.veryLongDuration * 2;
                    from: 0.2;
                    to: 1.0;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: Kirigami.Units.veryLongDuration;
                }
            }
        }
    }

    Item {
        id: progressWrapper
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: Plasmoid.configuration.showProgressBar
        implicitHeight: 1

        PlasmaComponents3.ProgressBar {
            id: remainingTimeProgressBar
            anchors.fill: parent

            from: Plasmoid.configuration.seconds
            to: 0
            value: fullRepresentation.seconds
        }
    }
}

