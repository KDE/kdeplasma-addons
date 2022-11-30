/*
 *   SPDX-FileCopyrightText: 2008, 2014 Davide Bettio <davide.bettio@kdemail.net>
 *   SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.kquickcontrolsaddons 2.0 as QtExtra

MouseArea {
    Layout.preferredWidth: Math.max(Plasmoid.compactRepresentationItem.width, PlasmaCore.Units.gridUnit * 10)
    Layout.preferredHeight: main.implicitHeight

    onClicked: root.toggleTimer()

    Component {
        id: popupHeadingComponent

        PlasmaExtras.PlasmoidHeading {
            leftPadding: PlasmaCore.Units.smallSpacing * 2
            rightPadding: PlasmaCore.Units.smallSpacing * 2

            contentItem: PlasmaExtras.Heading {
                level: 3
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                text: root.title
            }
        }
    }

    Component {
        id: desktopHeadingComponent

        PlasmaComponents3.Label {
            elide: Text.ElideRight
            font.pixelSize: 0.3 * timerDigits.height
            text: root.title
        }
    }

    ColumnLayout {
        id: main

        width: parent.width

        Loader {
            Layout.fillWidth: true

            active: root.showTitle

            sourceComponent: root.inPanel ? popupHeadingComponent : desktopHeadingComponent
        }

        TimerEdit {
            id: timerDigits

            Layout.fillWidth: true
            visible: root.showRemainingTime

            value: root.seconds
            editable: !root.running
            alertMode: root.alertMode
            onDigitModified: root.seconds += valueDelta
            SequentialAnimation on opacity {
                running: root.suspended;
                loops: Animation.Infinite;
                NumberAnimation {
                    duration: PlasmaCore.Units.veryLongDuration * 2;
                    from: 1.0;
                    to: 0.2;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: PlasmaCore.Units.veryLongDuration;
                }
                NumberAnimation {
                    duration: PlasmaCore.Units.veryLongDuration * 2;
                    from: 0.2;
                    to: 1.0;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: PlasmaCore.Units.veryLongDuration;
                }
            }
        }

        PlasmaComponents3.ProgressBar {
            id: remainingTimeProgressBar

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.showProgressBar

            from: plasmoid.configuration.seconds
            to: 0
            value: root.seconds
        }

        function resetOpacity() {
            timerDigits.opacity = 1.0;
        }

        Component.onCompleted: {
            root.opacityNeedsReset.connect(resetOpacity);
        }
    }
}

