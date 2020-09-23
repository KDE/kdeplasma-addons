/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9

import QtQuick.Window 2.2
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.5 as Kirigami


Window {
    id: dialog

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    flags: Qt.Dialog
    modality: Qt.WindowModal

    width: Kirigami.Units.gridUnit * 25
    height: Kirigami.Units.gridUnit * 20

    title: i18nc("@title:window", "Select Weather Station")
    color: syspal.window

    property alias selectedServices : stationPicker.selectedServices
    property alias source: stationPicker.source

    signal accepted

    function close() {
        dialog.visible = false;
    }


    SystemPalette {
        id: syspal
    }

    QQC2.Action {
        id: acceptAction

        shortcut: "Return"
        enabled: !!source && !stationPicker.handlesEnterKey
        onTriggered: {
            accepted();
            dialog.close();
        }
    }

    QQC2.Action {
        id: cancelAction

        shortcut: "Escape"
        onTriggered: {
            dialog.close();
        }
    }

    ColumnLayout {
        id: mainColumn
        anchors {
            fill: parent
            margins: mainColumn.spacing * Screen.devicePixelRatio //margins are hardcoded in QStyle we should match that here
        }

        WeatherStationPicker {
            id: stationPicker

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        RowLayout {
            id: buttonsRow

            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

            QQC2.Button {
                enabled: !!source
                icon.name: "dialog-ok"
                text: i18nc("@action:button", "Select")
                onClicked: {
                    acceptAction.trigger();
                }
            }
            QQC2.Button {
                icon.name: "dialog-cancel"
                text: i18nc("@action:button", "Cancel")
                onClicked: {
                    cancelAction.trigger();
                }
            }
        }
    }
}
