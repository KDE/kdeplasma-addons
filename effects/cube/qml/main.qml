/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick3D 1.15
import org.kde.kwin 3.0 as KWinComponents
import org.kde.kwin.effect.cube 1.0

Item {
    id: root
    focus: true

    required property QtObject effect
    required property QtObject targetScreen

    property bool animationEnabled: false

    function start() {
        cameraController.rotateTo(KWinComponents.Workspace.currentVirtualDesktop);
        root.animationEnabled = true;
        cameraController.state = "distant";
    }

    function stop() {
        cameraController.rotateTo(KWinComponents.Workspace.currentVirtualDesktop);
        cameraController.state = "close";
    }

    function switchToSelected() {
        const eulerRotation = effect.quaternionToEulerAngles(cameraController.rotation);
        const desktop = cube.desktopAt(eulerRotation.y);
        KWinComponents.Workspace.currentVirtualDesktop = desktop;
        effect.deactivate();
    }

    View3D {
        id: view
        anchors.fill: parent

        Loader {
            id: colorSceneEnvironment
            active: effect.backgroundMode == CubeEffect.BackgroundMode.Color
            sourceComponent: SceneEnvironment {
                clearColor: effect.backgroundColor
                backgroundMode: SceneEnvironment.Color
            }
        }

        Loader {
            id: skyboxSceneEnvironment
            active: effect.backgroundMode == CubeEffect.BackgroundMode.Skybox
            sourceComponent: SceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: Texture {
                    source: effect.skybox
                }
            }
        }

        environment: {
            switch (effect.backgroundMode) {
            case CubeEffect.BackgroundMode.Skybox:
                return skyboxSceneEnvironment.item;
            case CubeEffect.BackgroundMode.Color:
                return colorSceneEnvironment.item;
            }
        }

        PerspectiveCamera { id: camera }

        Cube {
            id: cube
            faceDisplacement: effect.cubeFaceDisplacement
            faceSize: Qt.size(root.width, root.height)
        }

        CubeCameraController {
            id: cameraController
            anchors.fill: parent
            state: "close"
            camera: camera
            xInvert: effect.mouseInvertedX
            yInvert: effect.mouseInvertedY

            states: [
                State {
                    name: "close"
                    PropertyChanges {
                        target: cameraController
                        radius: cube.faceDistance + 0.5 * cube.faceSize.height / Math.tan(0.5 * camera.fieldOfView * Math.PI / 180)
                    }
                },
                State {
                    name: "distant"
                    PropertyChanges {
                        target: cameraController
                        radius: cube.faceDistance * effect.distanceFactor + 0.5 * cube.faceSize.height / Math.tan(0.5 * camera.fieldOfView * Math.PI / 180)
                    }
                }
            ]

            Behavior on rotation {
                enabled: !cameraController.busy && root.animationEnabled
                QuaternionAnimation {
                    duration: effect.animationDuration
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on radius {
                NumberAnimation {
                    duration: effect.animationDuration
                    easing.type: Easing.OutCubic
                }
            }

            function rotateToLeft() {
                rotation = effect.quaternionDotProduct(Quaternion.fromEulerAngles(0, -cube.angleTick, 0), rotation);
            }

            function rotateToRight() {
                rotation = effect.quaternionDotProduct(Quaternion.fromEulerAngles(0, cube.angleTick, 0), rotation);
            }

            function rotateTo(desktop) {
                rotation = Quaternion.fromEulerAngles(0, cube.desktopAzimuth(desktop), 0);
            }
        }
    }

    MouseArea {
        anchors.fill: view
        onClicked: root.switchToSelected();
    }

    Keys.onEscapePressed: effect.deactivate();
    Keys.onLeftPressed: cameraController.rotateToLeft();
    Keys.onRightPressed: cameraController.rotateToRight();
    Keys.onEnterPressed: root.switchToSelected();
    Keys.onReturnPressed: root.switchToSelected();
    Keys.onSpacePressed: root.switchToSelected();

    Component.onCompleted: start();
}
