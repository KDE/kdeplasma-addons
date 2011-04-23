/*
 *   Copyright 2011 Anton Kreuzkamp <akreuzkamp@web.de>
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
    id:taskListView
    width: page.width; height: page.height

    function updateLists(data)
    {
        var index = Filter.tabIDs.indexOf(data.id)

        if (index == -1 && (plasmoid.readConfig("hideEmptyLists")==false || Filter.isEmpty(data.id) == false) ) { //New Item that is to be shown
            tabBar.insertTab(index, data.name)
            Filter.tabIDs[index==-1?(tabBar.count-1):index] = data.id

            if (tabBar.count == 1) {
                lists.currentChanged("List:"+data.id)
                plasmoid.busy = false
            }
        } else if (index != -1 && plasmoid.readConfig("hideEmptyLists")==true && Filter.isEmpty(data.id) == true)  { //Existing Item that is no longer to be shown
            Filter.tabIDs.splice(index,1)
            tabBar.removeTab(index)
        } else if (index != -1) { //Existing Item that is still to be shown => just update the name
            tabBar.setTabText(index,data.name)
        }
    }

    function removeList(id)
    {
        var index = Filter.tabIDs.indexOf(id)
        if (index != -1) {
            tabBar.removeTab(index)
            Filter.tabIDs.splice(index,1)
        }
    }

    PlasmaWidgets.TabBar {
        id : tabBar
        x: 2; width: parent.width - x*2
        tabBarShown: true

        onCurrentChanged: lists.currentChanged("List:"+Filter.tabIDs[currentIndex])
    }

    Rectangle {
        x: 2; y: 50
        width: taskListView.width - x*2; height: taskListView.height - y*2
        color: "transparent"

        Component {
            id: taskDelegate
            Item {
                x:5; width: parent.width - x*2
                height: 24
                PlasmaWidgets.Frame {
                    id: background
                    anchors.fill:parent
                    frameShadow: bgMouse.pressed ? "Sunken" : "Raised"
                    Text {
                        id: taskText
                        x: 4; y: 4
                        width: background.width - x*2
                        color: priority==1 ? "firebrick" : priority==2 ? "royalblue" : "black"
                        wrapMode: Text.NoWrap
                        clip: true
                        text: '<b>'+name+'</b>'
                    }
                    PlasmaWidgets.IconWidget {
                        id:completeButton
                        x:background.width-25
                        anchors.verticalCenter: background.verticalCenter
                        width:20
                        opacity:0
                        Component.onCompleted: setIcon("dialog-ok")
                        onClicked: {
                            var service = tasks.serviceForSource("Task:"+id)
                            var cg = service.operationDescription("setCompleted")
                            cg.completed = true
                            service.startOperationCall(cg)
                            lists.currentChanged("List:"+Filter.tabIDs[tabBar.currentIndex])
                        }
                    }
                    MouseArea {
                        id: bgMouse
                        width: background.width-20
                        height: background.height

                        onClicked: {
                            mainView.currentIndex=1
                            taskEditor.load(id)
                        }
                    }
                    MouseArea {
                        id: bgHover
                        anchors.fill: background
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton & Qt.RightButton //FIXME: very ugly
                    }
                }
                states: State {
                    name: "hovered"; when: bgHover.containsMouse
                    PropertyChanges { target: completeButton; opacity:1}
                    PropertyChanges { target: taskText; width: background.width - x*2 - 20}
                }

                transitions: Transition {
                    NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
                    NumberAnimation { properties: "width"; easing.type: Easing.InOutQuad }
                }
            }
        }

        ListView {
            anchors.fill: parent
            id: todoList
            model: PlasmaCore.DataModel {
                dataSource: tasks
                id: todoModel
            }
            delegate: taskDelegate
            focus: true
        }
    }
}