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

    property int minimumWidth: 364
    property int minimumHeight: 265

    anchors.fill: parent
    clip: true
    state: "fiveDays"

    PlasmaCore.Theme {
        id: theme
    }

    TopPanel {
        id: panel
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
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
        width: noticesView.visible ? 240 : 160
        height: 30
        visible: detailsView.model.length > 0

        PlasmaComponents.TabButton {
            text: weatherApplet.panelModel.totalDays
            onClicked: root.state = "fiveDays";
        }
        PlasmaComponents.TabButton {
            text: i18n("Details")
            onClicked: root.state = "details";
        }
        PlasmaComponents.TabButton {
            text: i18n("Notices")
            visible: noticesView.visible
            onClicked: root.state = "notices";
        }
    }

    FiveDaysView {
        id: fiveDaysView
        anchors {
            top: tabBar.visible ? tabBar.bottom : tabBar.top
            bottom: courtesyLabel.top
            topMargin: 14
            bottomMargin: 14
        }
        width: parent.width
        model: weatherApplet.fiveDaysModel
    }

    DetailsView {
        id: detailsView
        anchors {
            top: fiveDaysView.top
            bottom: fiveDaysView.bottom
        }
        width: parent.width
        model: weatherApplet.detailsModel
    }

    NoticesView {
        id: noticesView
        anchors {
            top: fiveDaysView.top
            bottom: fiveDaysView.bottom
        }
        width: parent.width
        model: weatherApplet.noticesModel
    }

    Text {
        id: courtesyLabel
        anchors {
            bottom: parent.bottom
            right: parent.right
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

    states: [
        State {
            name: "fiveDays"
            PropertyChanges { target: fiveDaysView; x: 0 }
            PropertyChanges { target: detailsView; x: root.width + 10 }
            PropertyChanges { target: noticesView; x: 2*root.width + 10 }
        },
        State {
            name: "details"
            PropertyChanges { target: fiveDaysView; x: -root.width - 10 }
            PropertyChanges { target: detailsView; x: 0 }
            PropertyChanges { target: noticesView; x: root.width + 10 }
        },
        State {
            name: "notices"
            PropertyChanges { target: fiveDaysView; x: -2*root.width - 10 }
            PropertyChanges { target: detailsView; x: -root.width - 10 }
            PropertyChanges { target: noticesView; x: 0 }
        }
    ]

    transitions: Transition {
        to: "*"
        PropertyAnimation { target: fiveDaysView; property: "x"; easing.type: Easing.InQuad; duration: 250 }
        PropertyAnimation { target: detailsView; property: "x"; easing.type: Easing.InQuad; duration: 250 }
        PropertyAnimation { target: noticesView; property: "x"; easing.type: Easing.InQuad; duration: 250 }
    }
}
