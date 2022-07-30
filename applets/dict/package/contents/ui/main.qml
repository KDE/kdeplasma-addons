/*
 *  SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0
import QtWebEngine 1.1

import org.kde.kirigami 2.19 as Kirigami

import org.kde.plasma.private.dict 1.0

ColumnLayout {
    Keys.forwardTo: input

    DictObject {
        id: dict
        selectedDictionary: plasmoid.configuration.dictionary
        // Activate the busy indicator, and deactivate it when page is loaded.
        onSearchInProgress: placeholder.opacity = 1;
        onDefinitionFound: {
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
            focus: Plasmoid.expanded && !Kirigami.InputMethod.willShowOnActive
            placeholderText: i18nc("@info:placeholder", "Enter word to define here…")
            Layout.fillWidth: true
            Layout.minimumWidth: PlasmaCore.Units.gridUnit * 12
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
            display: PlasmaComponents3.AbstractButton.IconOnly
            icon.name: "configure"
            text: Plasmoid.action("configure").text
            onClicked: plasmoid.action("configure").trigger();
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

            zoomFactor: PlasmaCore.Units.devicePixelRatio
            profile: dict.webProfile
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
                    duration: PlasmaCore.Units.veryLongDuration
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
                    width: parent.width - PlasmaCore.Units.gridUnit * 2 // For text wrap
                    anchors.centerIn: parent
                    iconName: "network-disconnect"
                    text: i18n("Unable to load definition")
                    explanation: i18nc("%2 human-readable error string", "Error code: %1 (%2)", dict.errorCode, dict.errorString)
                }
            }
        }
    }

}
