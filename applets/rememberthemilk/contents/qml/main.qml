/*
 *   Copyright 2010 Alexis Menard <menard@kde.org>
 *
 *   This program is free software you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
import "filter.js" as Filter

QGraphicsWidget {
    id: page
    preferredSize: "350x500"
    minimumSize: "350x250"

//---------------------------------------------------------------Data-Sources-------------------------------------
    Item {
        id:main

        PlasmaCore.DataSource {
            id: source
            engine: "rtm"
            connectedSources: ["Lists","Tasks","Auth"]
            property variant tabList: []

            onDataChanged: {
                if (data["Auth"]["ValidToken"]) {
                    authMessage.opacity=0
                }
                var same = true
                for (i in data["Lists"]) {
                    if (data["Lists"][i] != tabList[i]) {
                        same = false
                        break
                    }
                }
                if (!same) {
                    tabList = data["Lists"]
                    while (tabBar.count)
                        tabBar.removeTab(0)
                    for (var i in data["Lists"]) {
                        tabBar.addTab(data["Lists"][i])
                        Filter.tabIDs[(tabBar.count-1)] = i
                        lists.connectSource("List:" + i)
                    }
                }
            }
        }

        PlasmaCore.DataSource {
            id: lists
            engine: "rtm"

            onDataChanged: {
                for (s in data) {
                    if (!Filter.isCurrentList(data[s].id)) {
                        for (i in data[s]) {
                            if (i != "smart" && i != "filter" && i != "id" && i != "name") {
                                tasks.disconnectSource("Task:"+i)
                            }
                        }
                    } else {
                        for (i in data[s]) {
                            if (i != "smart" && i != "filter" && i != "id" && i != "name") {
                                tasks.connectSource("Task:"+i)
                            }
                        }
                    }
                }
            }
        }

        PlasmaCore.DataSource {
            id: tasks
            engine: "rtm"
            property variant taskList: []

            onDataChanged: {
                same = true
                for (s in data) {
                    if (data[s].name != taskList[i]) {
                        same = false
                        break
                    }
                }
                if (!same) {
                    todoList.clear()
                    for (s in data) {
                        taskList[s] = data[s].name
                        todoList.append( {"todo": data[s].name, "priority":data[s].priority} )
                    }
                    plasmoid.busy = false
                }
            }
        }
    }

    Component.onCompleted: {
        var authService = source.serviceForSource("Auth")

        var token = plasmoid.readConfig("token")
        if (!token) {
            debug("token not found.")
            authMessage.opacity=1
        } else {
            var cg = authService.operationDescription("AuthWithToken")
            cg.token=token
            authService.startOperationCall(cg)
            tabBar.addTab("Loading...")
            plasmoid.busy = true
        }
    }

    PlasmaWidgets.TabBar {
            id : tabBar
            x: 2; width: parent.width - x*2
            anchors.topMargin : main
            tabBarShown: true
    }

//---------------------------------------------------------------Todo-List-------------------------------------
    Rectangle {
        x: 2; y: 50
        width: page.width - x*2; height: page.height - y*2
        anchors.bottom: page
        color: "transparent"

        Component {
            id: contactDelegate
            Item {
                x:5; width: parent.width - x*2
                height: bgMouse.containsMouse ? taskText.paintedHeight + 8 : 24
                Rectangle {
                    id: background
                    x: 2; y: 2; width: parent.width - x*2; height: parent.height - y*2
                    color: priority==1 ? "firebrick" : priority==2 ? "royalblue" : "aliceblue"
                    border.color: "skyblue"
                    radius: 5
                    Text {
                        id: taskText
                        x: 4; y: 4
                        width: parent.width - x*2
                        wrapMode: bgMouse.containsMouse ? Text.Wrap : Text.NoWrap
                        clip: true
                        text: '<b>'+todo+'</b>'
                    }
                    MouseArea {
                        id: bgMouse
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                }
            }
        }

        ListView {
            anchors.fill: parent
            model: ListModel { id:todoList }
            delegate: contactDelegate
            focus: true
        }
    }

//---------------------------------------------------------Authentication Required-------------------------------
    Rectangle {
        id: authMessage
        anchors.fill: page
        color: "#55aaaaaa"
        opacity:0

        PlasmaWidgets.PushButton {
            text: "Authenticate"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            onClicked: {
                var authService = source.serviceForSource("Auth")
                var cg = authService.operationDescription("Login")
                authService.startOperationCall(cg)
            }
        }
    }

//---------------------------------------------------------------Task-Editor-------------------------------------
    Rectangle {
        x: 2; y: 50
        width: page.width - x*2; height: page.height - y*2
        anchors.bottom: page
        opacity: 0
    }
}