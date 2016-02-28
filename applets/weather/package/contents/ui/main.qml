/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root

    property int implicitWidth: 250
    property int implicitHeight: 350

    property int minimumWidth: 373
    property int minimumHeight: 272

    Plasmoid.icon: plasmoid.nativeInterface.currentWeatherIconName
    Plasmoid.toolTipMainText: plasmoid.nativeInterface.currentWeatherToolTipMainText
    Plasmoid.toolTipSubText: plasmoid.nativeInterface.currentWeatherToolTipSubText

    Plasmoid.compactRepresentation: Component {
        MouseArea {
            id: compactRoot
            onClicked: plasmoid.expanded = !plasmoid.expanded

            PlasmaCore.IconItem {
                width: height
                height: compactRoot.height
                source: plasmoid.nativeInterface.currentWeatherIconName
            }
        }
    }

    Plasmoid.fullRepresentation: Item {
        id: fullRoot

        TopPanel {
            id: panel
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                // matching round ends of bars behind data rows
                margins: units.smallSpacing
            }
            height: parent.height * 0.21
            model: plasmoid.nativeInterface.panelModel
        }

        PlasmaComponents.TabBar {
            id: tabBar
            anchors {
                top: panel.bottom
                topMargin: units.smallSpacing
                horizontalCenter: parent.horizontalCenter
            }

            visible: detailsView.model.length > 0

            PlasmaComponents.TabButton {
                text: plasmoid.nativeInterface.panelModel.totalDays
                tab: fiveDaysView
            }
            PlasmaComponents.TabButton {
                text: i18n("Details")
                tab: detailsView
            }
            PlasmaComponents.TabButton {
                text: i18n("Notices")
                visible: noticesView.visible
                onClicked: noticesView
            }
        }

        PlasmaComponents.TabGroup {
            id: mainTabGroup
            anchors {
                top: tabBar.visible ? tabBar.bottom : tabBar.top
                bottom: courtesyLabel.top
                topMargin: units.largeSpacing
                bottomMargin: units.largeSpacing
            }
            width: panel.width
            FiveDaysView {
                id: fiveDaysView
                anchors.fill: parent
                model: plasmoid.nativeInterface.fiveDaysModel
            }

            DetailsView {
                id: detailsView
                anchors.fill: parent
                model: plasmoid.nativeInterface.detailsModel
            }

            NoticesView {
                id: noticesView
                anchors.fill: parent
                model: plasmoid.nativeInterface.noticesModel
            }
        }

        PlasmaComponents.Label {
            id: courtesyLabel
            anchors {
                bottom: parent.bottom
                right: parent.right
                bottomMargin: unit.smallSpacing
            }
            font {
                pointSize: theme.smallestFont.pointSize
                underline: mouseArea.enabled
            }
            text: plasmoid.nativeInterface.panelModel.courtesy

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                enabled: !!plasmoid.nativeInterface.panelModel.creditUrl
                onClicked: Qt.openUrlExternally(plasmoid.nativeInterface.panelModel.creditUrl);
            }
        }
    }
}
