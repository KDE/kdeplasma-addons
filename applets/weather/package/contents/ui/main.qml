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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: root

    property int minimumWidth: 373
    property int minimumHeight: 272

    anchors.fill: parent
    clip: true

    PlasmaCore.Theme {
        id: theme
    }

    TopPanel {
        id: panel
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 5
        }
        height: parent.height * 0.21
        model: weatherApplet.panelModel
    }

    PlasmaComponents.TabBar {
        id: tabBar
        anchors {
            top: panel.bottom
            topMargin: 5
            horizontalCenter: parent.horizontalCenter
        }

        visible: detailsView.model.length > 0

        PlasmaComponents.TabButton {
            text: weatherApplet.panelModel.totalDays
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
            topMargin: 12
            bottomMargin: 15
        }
        width: panel.width
        FiveDaysView {
            id: fiveDaysView
            anchors.fill: parent
            model: weatherApplet.fiveDaysModel
        }

        DetailsView {
            id: detailsView
            anchors.fill: parent
            model: weatherApplet.detailsModel
        }

        NoticesView {
            id: noticesView
            anchors.fill: parent
            model: weatherApplet.noticesModel
        }
    }

    Text {
        id: courtesyLabel
        anchors {
            bottom: parent.bottom
            right: parent.right
            bottomMargin: 7
        }
        font {
            pointSize: theme.smallestFont.pointSize
            underline: mouseArea.enabled
        }
        color: theme.textColor
        text: weatherApplet.panelModel.courtesy

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            enabled: weatherApplet.panelModel.enableLink
            onClicked: weatherApplet.invokeBrowser();
        }
    }
}
