/************************************************************************************
 * Copyright (C) 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>                *
 *                                                                                  *
 * This program is free software; you can redistribute it and/or                    *
 * modify it under the terms of the GNU General Public License                      *
 * as published by the Free Software Foundation; either version 2                   *
 * of the License, or (at your option) any later version.                           *
 *                                                                                  *
 * This program is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 * GNU General Public License for more details.                                     *
 *                                                                                  *
 * You should have received a copy of the GNU General Public License                *
 * along with this program; if not, write to the Free Software                      *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 ************************************************************************************/

import QtQuick 2.2
import QtQuick.Layouts 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
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
        height: units.gridUnit * 16
        width: units.gridUnit * 16

        PlasmaExtras.Heading {
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
            delegate: PlasmaComponents.ListItem {
                height: button.height * 1.2
                enabled: true

                onClicked: view.createJob(index)

                RowLayout {
                    anchors.fill: parent
                    PlasmaComponents.Label {
                        Layout.fillWidth: true
                        text: display
                    }
                    PlasmaComponents.Button {
                        id: button
                        anchors.verticalCenter: parent.verticalCenter
                        iconSource: "arrow-right"
                        onClicked: view.createJob(index)
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
