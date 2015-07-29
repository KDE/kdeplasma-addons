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
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as Components
// import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kio 1.0 as Kio

import org.kde.plasma.private.quotawatch 0.1

Item {
    Component.onCompleted: plasmoid.removeAction("configure")
//     Plasmoid.status: quotaMonitor.critical ? PlasmaCore.Types.NeedsAttentionStatus :
//    Plasmoid.status: PlasmaCore.Types.ActiveStatus
//    Plasmoid.status: PlasmaCore.Types.PassiveStatus
   Plasmoid.status: PlasmaCore.Types.NeedsAttentionStatus

    Layout.minimumWidth: units.gridUnit * 14
    Layout.minimumHeight: units.gridUnit * 4

    Plasmoid.switchWidth: units.gridUnit * 10
    Plasmoid.switchHeight: units.gridUnit * 10

    QuotaWatch {
        id: quotaMonitor
    }

    Plasmoid.compactRepresentation: MouseArea {
        Components.Label {
            text: quotaMonitor.status
        }
        onClicked: plasmoid.expanded = !plasmoid.expanded
    }

    Plasmoid.fullRepresentation: Item {
        id: root

        width: units.gridUnit * 14
        height: units.gridUnit * 14

        ListView {
            id: listView
            anchors.fill: root
            model: quotaMonitor.quotaItems
            boundsBehavior: Flickable.StopAtBounds
            highlight: Components.Highlight { }
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            currentIndex: -1
            delegate: ListDelegateItem {
                width: listView.width
                mountPoint: model.mountString
                details: model.detailString
                usage: model.usage
            }
        }
    }

    Kio.KRun {
        id: kRun
    }

    Plasmoid.toolTipMainText: i18n("File System Quota")
    Plasmoid.toolTipSubText: quotaMonitor.toolTip
}
