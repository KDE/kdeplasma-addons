/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Controls 1.1 as QtControls
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents // for Highlight
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kcoreaddons 1.0 as KCoreAddons // kuser
import org.kde.kquickcontrolsaddons 2.0 // kcmshell

import org.kde.plasma.private.sessions 2.0 as Sessions

Item {
    id: root

    readonly property bool isVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property string displayedName: showFullName ? kuser.fullName : kuser.loginName

    readonly property bool showFace: plasmoid.configuration.showFace
    readonly property bool showName: plasmoid.configuration.showName

    readonly property bool showFullName: plasmoid.configuration.showFullName

    // TTY number and X display
    readonly property bool showTechnicalInfo: plasmoid.configuration.showTechnicalInfo

    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 10
    Plasmoid.switchHeight: PlasmaCore.Units.gridUnit * 12

    Plasmoid.toolTipTextFormat: Text.StyledText
    Plasmoid.toolTipSubText: i18n("You are logged in as <b>%1</b>", displayedName)

    Binding {
        target: plasmoid
        property: "icon"
        value: kuser.faceIconUrl
        // revert to the plasmoid icon if no face given
        when: kuser.faceIconUrl.toString() !== ""
    }

    KCoreAddons.KUser {
        id: kuser
    }

    Plasmoid.compactRepresentation: MouseArea {
        id: compactRoot

        // Taken from DigitalClock to ensure uniform sizing when next to each other
        readonly property bool tooSmall: plasmoid.formFactor === PlasmaCore.Types.Horizontal && Math.round(2 * (compactRoot.height / 5)) <= PlasmaCore.Theme.smallestFont.pixelSize

        Layout.minimumWidth: isVertical ? 0 : compactRow.implicitWidth
        Layout.maximumWidth: isVertical ? Infinity : Layout.minimumWidth
        Layout.preferredWidth: isVertical ? undefined : Layout.minimumWidth

        Layout.minimumHeight: isVertical ? label.height : PlasmaCore.Theme.smallestFont.pixelSize
        Layout.maximumHeight: isVertical ? Layout.minimumHeight : Infinity
        Layout.preferredHeight: isVertical ? Layout.minimumHeight : PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 2

        onClicked: plasmoid.expanded = !plasmoid.expanded

        Row {
            id: compactRow

            anchors.centerIn: parent
            spacing: PlasmaCore.Units.smallSpacing

            PlasmaCore.IconItem {
                id: icon
                width: height
                height: compactRoot.height
                Layout.preferredWidth: height
                source: visible ? (kuser.faceIconUrl.toString() || "user-identity") : ""
                visible: root.showFace
                usesPlasmaTheme: false
            }

            PlasmaComponents3.Label {
                id: label
                text: root.displayedName
                height: compactRoot.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.NoWrap
                fontSizeMode: Text.VerticalFit
                font.pixelSize: tooSmall ? PlasmaCore.Theme.defaultFont.pixelSize : PlasmaCore.Units.roundToIconSize(PlasmaCore.Units.gridUnit * 2)
                minimumPointSize: PlasmaCore.Theme.smallestFont.pointSize
                visible: root.showName
            }
        }
    }

    Plasmoid.fullRepresentation: Item {
        id: fullRoot

        Layout.preferredWidth: PlasmaCore.Units.gridUnit * 12
        Layout.preferredHeight: Math.min(Screen.height * 0.5, column.contentHeight)

        PlasmaCore.DataSource {
            id: pmEngine
            engine: "powermanagement"
            connectedSources: ["PowerDevil", "Sleep States"]

            function performOperation(what) {
                var service = serviceForSource("PowerDevil")
                var operation = service.operationDescription(what)
                service.startOperationCall(operation)
            }
        }

        Sessions.SessionsModel {
            id: sessionsModel
        }

        PlasmaComponents.Highlight {
            id: delegateHighlight
            visible: false
            z: -1 // otherwise it shows ontop of the icon/label and tints them slightly
        }

        ColumnLayout {
            id: column

            // there doesn't seem a more sensible way of getting this due to the expanding ListView
            readonly property int contentHeight: currentUserItem.height + userList.contentHeight + PlasmaCore.Units.smallSpacing
                                               + (newSessionButton.visible ? newSessionButton.height : 0)
                                               + (lockScreenButton.visible ? lockScreenButton.height : 0)
                                               + leaveButton.height

            anchors.fill: parent

            spacing: 0

            ListDelegate {
                id: currentUserItem
                text: root.displayedName
                subText: i18n("Current user")
                icon: kuser.faceIconUrl.toString() || "user-identity"
                interactive: false
                interactiveIcon: KCMShell.authorize("kcm_users.desktop").length > 0
                onIconClicked: KCMShell.openSystemSettings("kcm_users")
                usesPlasmaTheme: false
            }

            PlasmaExtras.ScrollArea {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ListView {
                    id: userList
                    model: sessionsModel

                    highlight: PlasmaComponents.Highlight {}
                    highlightMoveDuration: 0

                    delegate: ListDelegate {
                        width: userList.width
                        text: {
                            if (!model.session) {
                                return i18nc("Nobody logged in on that session", "Unused")
                            }

                            if (model.realName && root.showFullName) {
                                return model.realName
                            }

                            return model.name
                        }
                        icon: model.icon || "user-identity"
                        subText: {
                            if (!root.showTechnicalInfo) {
                                return ""
                            }

                            if (model.isTty) {
                                return i18nc("User logged in on console number", "TTY %1", model.vtNumber)
                            } else if (model.displayNumber) {
                                return i18nc("User logged in on console (X display number)", "on %1 (%2)", model.vtNumber, model.displayNumber)
                            }
                            return ""
                        }

                        onClicked: sessionsModel.switchUser(model.vtNumber, sessionsModel.shouldLock)
                        onContainsMouseChanged: {
                            if (containsMouse) {
                                userList.currentIndex = index
                            } else {
                                userList.currentIndex = -1
                            }
                        }
                        usesPlasmaTheme: false
                    }
                }
            }

            ListDelegate {
                id: newSessionButton
                text: i18nc("@action", "New Session")
                icon: "system-switch-user"
                highlight: delegateHighlight
                visible: sessionsModel.canStartNewSession
                onClicked: sessionsModel.startNewSession(sessionsModel.shouldLock)
            }

            ListDelegate {
                id: lockScreenButton
                text: i18nc("@action", "Lock Screen")
                icon: "system-lock-screen"
                highlight: delegateHighlight
                enabled: pmEngine.data["Sleep States"]["LockScreen"]
                visible: enabled
                onClicked: pmEngine.performOperation("lockScreen")
            }

            ListDelegate {
                id: leaveButton
                text: i18nc("Show a dialog with options to logout/shutdown/restart", "Leave...")
                highlight: delegateHighlight
                icon: "system-shutdown"
                onClicked: pmEngine.performOperation("requestShutDown")
            }
        }

        Component.onCompleted: {
            plasmoid.expandedChanged.connect(function (expanded) {
                if (expanded) {
                    sessionsModel.reload();
                }
            });
        }
    }
}
