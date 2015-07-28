/*
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
import QtQuick 2.1
import QtQuick.Controls 1.1 as QtControls
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
// import org.kde.plasma.components 2.0 as PlasmaComponents
// import org.kde.plasma.calendar 2.0
// import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kio 1.0 as Kio

import org.kde.plasma.private.quotawatch 0.1

Item {

    Layout.minimumWidth: units.gridUnit * 4
    Layout.minimumHeight: units.gridUnit * 4

    Plasmoid.switchWidth: units.gridUnit * 10
    Plasmoid.switchHeight: units.gridUnit * 10

    QuotaWatch {
        id: quotaMonitor
    }

    Plasmoid.compactRepresentation: QtControls.Label {
        QtControls.Label { text: ":o" }
    }
    Plasmoid.fullRepresentation: QtControls.Label {
        id: root

        text: quotaMonitor.status

        width: units.gridUnit * 14
        height: units.gridUnit * 14


//         Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    }
    
    Kio.KRun {
        id: kRun
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        active: true
        mainText: i18n("Quota Monitor")
        subText: quotaMonitor.toolTip

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                //kRun.openUrl(apps.data["org.kde.filelight.desktop"].entryPath)
            }
        }
    }
}
