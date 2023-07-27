/*
 * SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Controls as QQC2
import QtQuick.Layouts 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.purpose 1.0 as Purpose

PlasmaCore.Dialog
{
    id: window
    flags: Qt.WindowStaysOnTopHint
    property var inputData
    property bool running: false
    signal finished(var output, int error, string message)

    hideOnWindowDeactivate: true
    mainItem: ColumnLayout {
        height: Kirigami.Units.gridUnit * 17
        width: Kirigami.Units.gridUnit * 16

        Kirigami.Heading {
            id: title
            Layout.fillWidth: true
            text: window.inputData.mimeType ? i18n("Shares for '%1'", window.inputData.mimeType) : ""
        }
        Purpose.AlternativesView {
            id: view
            Layout.fillWidth: true
            Layout.fillHeight: true
            inputData: window.inputData
            pluginType: "Export"
            Component {
                id: highlightComponent
                PlasmaExtras.Highlight {}
            }
            Component.onCompleted: {
                if (view.hasOwnProperty("highlight")) { //Purpose 1.1 doesn't have highlight
                    view.highlight = highlightComponent
                }
            }

            delegate: PlasmaExtras.ListItem {
                enabled: true

                onClicked: view.createJob(model.index)
                Keys.onReturnPressed: clicked(null)
                Keys.onEnterPressed: clicked(null)
                Keys.onSpacePressed: clicked(null)
                Keys.onEscapePressed: window.hide()

                implicitHeight: contentLayout.implicitHeight + Kirigami.Units.smallSpacing * 2

                RowLayout {
                    id: contentLayout
                    anchors.left: parent.left
                    anchors.right: parent.right
                    QQC2.Label {
                        Layout.fillWidth: true
                        text: model.display
                    }
                    PlasmaComponents3.Button {
                        id: button
                        Layout.alignment: Qt.AlignVCenter
                        icon.name: "arrow-right"
                        onClicked: view.createJob(model.index)
                    }
                }
            }
            onVisibleChanged: {
                if (visible) {
                    view.reset();
                }
            }

            onRunningChanged: window.running = running
            onFinished: {
                window.finished(output, error, message)
            }
        }
    }
}
