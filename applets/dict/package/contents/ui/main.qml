/*
 *  SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0
import QtWebEngine
import QtQuick.Controls

import org.kde.plasma.private.dict 1.0

PlasmoidItem {
    id: root
    // panels don't take keyboard focus when you click them, so the fullRepresentation in a very thick
    // panel is worse than useless as the user can't really do anything with it. If that changes, allow
    // the widget to switch to fullRepresentation in panels too.
    switchWidth: inPanel ? -1 : Kirigami.Units.gridUnit * 10
    switchHeight: inPanel ? -1 : Kirigami.Units.gridUnit * 10
    Plasmoid.icon: "accessories-dictionary-symbolic"

    property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    fullRepresentation: ColumnLayout {
        Keys.forwardTo: input

        DictObject {
            id: dict
            selectedDictionary: plasmoid.configuration.dictionary
            // Activate the busy indicator, and deactivate it when page is loaded.
            onSearchInProgress: placeholder.opacity = 1;
            onDefinitionFound: html => {
                web.loadHtml(html);
                placeholder.opacity = 0;
            }
        }

        RowLayout {
            focus: true
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            PlasmaExtras.SearchField {
                id: input
                focus: root.expanded && !Kirigami.InputMethod.willShowOnActive
                placeholderText: i18nc("@info:placeholder", "Enter word to define here…")
                Layout.fillWidth: true
                Layout.minimumWidth: Kirigami.Units.gridUnit * 12
                onAccepted: {
                    if (input.text === "") {
                        web.visible = false;
                        placeholder.opacity = 0;
                        web.loadHtml("");
                    } else {
                        web.visible = Qt.binding(() => !dict.hasError);
                        dict.lookup(input.text);
                    }
                }
            }
            PlasmaComponents3.Button {
                id: configureButton

                display: PlasmaComponents3.AbstractButton.IconOnly
                hoverEnabled: true
                icon.name: "configure"
                text: Plasmoid.internalAction("configure").text

                PlasmaComponents3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PlasmaComponents3.ToolTip.text: configureButton.text
                PlasmaComponents3.ToolTip.visible: configureButton.hovered

                onClicked: Plasmoid.internalAction("configure").trigger();
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: input.Layout.minimumWidth

            WebEngineView {
                id: web
                anchors.fill: parent
                visible: false

                zoomFactor: 1
                profile: dict.webProfile
                property Menu contextMenu: Menu {
                    Repeater {
                        model: [
                            WebEngineView.Back,
                            WebEngineView.Forward,
                            WebEngineView.Copy,
                        ]
                        MenuItem {
                            text: web.action(modelData).text
                            enabled: web.action(modelData).enabled
                            onClicked: web.action(modelData).trigger()
                            icon.name: web.action(modelData).iconName
                            display: MenuItem.TextBesideIcon
                        }
                    }
                }
                onContextMenuRequested: request => {
                    request.accepted = true;
                    contextMenu.popup();
                }
            }

            Item {
                id: placeholder
                anchors.fill: parent
                opacity: 0

                Loader {
                    active: placeholder.visible
                    anchors.fill: parent
                    asynchronous: true

                    sourceComponent: dict.hasError ? errorPlaceholder : loadingPlaceholder
                }

                Behavior on opacity {
                    NumberAnimation {
                        easing.type: Easing.InOutQuad
                        duration: Kirigami.Units.veryLongDuration
                    }
                }
            }

            Component {
                id: loadingPlaceholder

                Rectangle {
                    anchors.fill: parent
                    color: web.backgroundColor

                    PlasmaComponents3.BusyIndicator {
                        anchors.centerIn: parent
                    }
                }
            }

            Component {
                id: errorPlaceholder

                Item {
                    anchors.fill: parent

                    PlasmaExtras.PlaceholderMessage {
                        width: parent.width - Kirigami.Units.gridUnit * 2 // For text wrap
                        anchors.centerIn: parent
                        iconName: "network-disconnect"
                        text: i18n("Unable to load definition")
                        explanation: i18nc("%2 human-readable error string", "Error code: %1 (%2)", dict.errorCode, dict.errorString)
                    }
                }
            }
        }
    }
}
