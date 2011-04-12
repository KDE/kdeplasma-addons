/*
 *   Copyright 2010 Anton Kreuzkamp <akreuzkamp@web.de>
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
            id: sources
            engine: "rtm"
            connectedSources: ["Lists","Auth"]
            interval: 30000

            onNewData: {
                switch (sourceName) {
                case "Auth":
                    if ( data["ValidToken"]) {
                        authMessage.opacity=0
                        plasmoid.writeConfig("token", data["Token"])
                    }
                    break

                case "Lists":
                    for (i in data) {
                        lists.connectSource("List:"+i)
                    }
                    break
                }
            }
        }

        PlasmaCore.DataSource {
            id: lists
            engine: "rtm"
            interval: 30000

            onSourceDisconnected: {
                var index = Filter.tabIDs.indexOf(source.slice(5))
                if (index != -1) {
                    tabBar.removeTab(index)
                    Filter.tabIDs.splice(index,1)
                }
            }

            onNewData: {
                    var id = sourceName.slice(5)
                    var index = Filter.tabIDs.indexOf(id)
                    tabBar.removeTab(index)
                    if (plasmoid.readConfig("hideEmptyLists")==false || Filter.isEmpty(id) == false) {
                        tabBar.insertTab(index, data.name)
                        Filter.tabIDs[index==-1?(tabBar.count-1):index] = id

                        if (tabBar.count == 1) {
                            currentChanged(sourceName)
                            plasmoid.busy = false
                        }
                    } else  {
                        if (index != -1)
                            Filter.tabIDs.splice(index,1)
                    }
            }

            function currentChanged(newList)
            {
                for (i in tasks.connectedSources)
                    tasks.disconnectSource(tasks.connectedSources[0])

                for (i in data[newList]) {
                    if (i != "smart" && i != "filter" && i != "id" && i != "name")
                        tasks.connectSource("Task:"+i)
                }
            }
        }

        PlasmaCore.DataSource {
            id: tasks
            interval: 60000
            engine: "rtm"
        }
    }

    Component.onCompleted: {
        var authService = sources.serviceForSource("Auth")

        var token = plasmoid.readConfig("token")
        if (token=="") {
            debug("token not found.")
            authMessage.opacity=1
        } else {
            var cg = authService.operationDescription("AuthWithToken")
            cg.token=token
            authService.startOperationCall(cg)
            plasmoid.busy = true
        }
    }

    PlasmaWidgets.TabBar {
            id : tabBar
            x: 2; width: parent.width - x*2
            anchors.topMargin : main
            tabBarShown: true

            onCurrentChanged: lists.currentChanged("List:"+Filter.tabIDs[currentIndex])
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
                //FIXME: nice idea, doesn't seem to behave too well
                //height: bgMouse.containsMouse ? taskText.paintedHeight + 8 : 24
                height: 24
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
                        text: '<b>'+name+'</b>'
                    }
                    MouseArea {
                        id: bgMouse
                        anchors.fill: parent
                        hoverEnabled: true

                        onClicked: {
                            taskEditor.showEditor(id)
                        }
                    }
                }
            }
        }

        ListView {
            anchors.fill: parent
            model: PlasmaCore.DataModel {
                dataSource: tasks
                id: todoList
            }
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
        id: taskEditor
        anchors.fill: page
        opacity: 0
        color: "#55aaaaaa"
        property string task

        Text {
            color: "#ff000000"
            text: "Name:"
        }

        PlasmaWidgets.LineEdit {
            id: nameEdit
        }

        function showEditor(taskID) {
            opacity=1
            nameEdit.text = currentList.data[taskID].name
        }
    }
}