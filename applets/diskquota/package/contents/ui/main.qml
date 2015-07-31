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
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.diskquota 1.0

Item {
    id: quotaApplet
    Component.onCompleted: plasmoid.removeAction("configure")
    Plasmoid.status: (diskQuota.status === DiskQuota.ActiveStatus) ? PlasmaCore.Types.ActiveStatus : ((diskQuota.status === DiskQuota.PassiveStatus) ? PlasmaCore.Types.PassiveStatus : PlasmaCore.Types.NeedsAttentionStatus)

    Layout.minimumWidth: units.gridUnit * 10
    Layout.minimumHeight: units.gridUnit * 2

    Plasmoid.switchWidth: units.gridUnit * 10
    Plasmoid.switchHeight: units.gridUnit * 10

    DiskQuota {
        id: diskQuota
    }

    Plasmoid.icon: diskQuota.iconName

    Plasmoid.fullRepresentation: Item {
        id: root

        width: units.gridUnit * 20
        height: units.gridUnit * 14

        // HACK: connection to reset currentIndex to -1. Without this, when
        // uninstalling filelight, the selection highlight remains fixed (which is wrong)
        Connections {
            target: diskQuota
            onCleanUpToolInstalledChanged: {
                if (!diskQuota.cleanUpToolInstalled) {
                    listView.currentIndex = -1
                }
            }
        }

        ColumnLayout {
            anchors.fill: root
            Components.Label {
                visible: ! diskQuota.quotaInstalled
                anchors.left: parent.left
                anchors.right: parent.right
                text: i18n("Quota tool not found. Please install 'quota'.")
                horizontalAlignment: Text.AlignLeft
            }

            PlasmaExtras.ScrollArea {
                anchors.fill: parent
                ListView {
                    id: listView
                    model: diskQuota.model()
                    boundsBehavior: Flickable.StopAtBounds
                    highlight: Components.Highlight { }
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0
                    currentIndex: -1
                    delegate: ListDelegateItem {
                        enabled: diskQuota.cleanUpToolInstalled
                        width: listView.width
                        mountPoint: model.mountPoint
                        details: model.details
                        iconName: model.icon
                        usedString: model.used
                        freeString: model.free
                        usage: model.usage
                    }
                }
            }
        }
    }

    Plasmoid.toolTipMainText: diskQuota.toolTip
    Plasmoid.toolTipSubText: diskQuota.subToolTip
}
