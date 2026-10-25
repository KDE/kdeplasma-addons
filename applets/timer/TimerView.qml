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

MouseArea {
    id: fullRepresentation

    required property bool inPanel
    required property bool isVertical
    required property bool running
    required property bool suspended
    required property bool alertMode
    required property int seconds

    signal toggleTimerRequested
    signal adjustTimerRequested(int delta)

    Layout.preferredWidth: inPanel ? Math.max(root.compactRepresentationItem?.width ?? 0, Kirigami.Units.gridUnit * 20) : -1
    Layout.preferredHeight: inPanel ? Math.round(Layout.preferredWidth * 2/5) : -1
    Layout.minimumHeight: inPanel ? Layout.preferredHeight : 0

    onClicked: fullRepresentation.toggleTimerRequested()

    Component {
        id: popupHeadingComponent

        PlasmaExtras.PlasmoidHeading {
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
            elide: Text.ElideRight
            fontSizeMode: Text.VerticalFit
            minimumPixelSize: 8
            font.pixelSize: height * 0.7
            text: Plasmoid.configuration.title
            textFormat: Text.PlainText
        }
    }

    ColumnLayout {
        id: main

        implicitHeight: (headerLoader.active ? (headerLoader.item as Item).implicitHeight : 0) + timerDigits.implicitHeight + (remainingTimeProgressBar.visible ? remainingTimeProgressBar.implicitHeight : 0)

        width: parent.width
        height: parent.height

        Loader {
            id: headerLoader
            Layout.minimumHeight: fullRepresentation.inPanel ? -1 : Kirigami.Units.gridUnit * 2
            Layout.maximumHeight: fullRepresentation.inPanel ? -1 : Math.round(main.height / 3)
            Layout.fillHeight: fullRepresentation.inPanel ? false : true
            Layout.fillWidth: true

            active: Plasmoid.configuration.showTitle
            visible: active

            sourceComponent: fullRepresentation.inPanel ? popupHeadingComponent : desktopHeadingComponent
        }

        TimerEdit {
            id: timerDigits

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.verticalStretchFactor: 3
            visible: Plasmoid.configuration.showRemainingTime

            value: fullRepresentation.seconds
            editable: !fullRepresentation.running
            alertMode: fullRepresentation.alertMode
            showSeconds: Plasmoid.configuration.showSeconds
            onDigitModified: valueDelta => fullRepresentation.adjustTimerRequested(valueDelta)
            onToggleTimerRequested: fullRepresentation.toggleTimerRequested()
            maximumHeight: height
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

        PlasmaComponents3.ProgressBar {
            id: remainingTimeProgressBar

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: Plasmoid.configuration.showProgressBar

            from: Plasmoid.configuration.seconds
            to: 0
            value: fullRepresentation.seconds
        }
    }
}

