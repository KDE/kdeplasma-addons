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

QGraphicsWidget {
    id: taskEditor
    width: page.width; height: page.height
    property string taskID

    function load(id) {
        taskID = id
        var task=tasks.data["Task:"+taskID]

        nameEdit.text = task.name

        tagEdit.text = ""
        for (i=0; i<task.tags.length; i++) {
            tagEdit.text += task.tags[i]
            if (task.tags.length-i!=1) //Don't set a comma after the last tag
                tagEdit.text += ","
        }

//         dateEdit.text = task.due //FIXME: Cannot assign QDateTime to QString

        priorityBox.currentIndex = task.priority -1
    }

    Grid {
        anchors.fill: parent
        columns: 2
        spacing: 10
        Item {
            width:100
            height:30
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: "Name:"
            }
        }
        Item {
            width:page.width - 120
            height:30
            PlasmaWidgets.LineEdit { width:parent.width; id: nameEdit }
        }

        Item {
            width:100
            height:30
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: "Tags:"
            }
        }
        Item {
            width:page.width - 120
            height:30
            PlasmaWidgets.LineEdit { width:parent.width; id: tagEdit }
        }

        Item {
            width:100
            height:30
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: "Due:"
            }
        }
        Item {
            width:page.width - 120
            height:30
            PlasmaWidgets.LineEdit { width:parent.width; id: dateEdit }
        }

        Item {
            width:100
            height:30
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: "Priority:"
            }
        }
        Item {
            width:page.width - 120
            height:30
            PlasmaWidgets.ComboBox {
                width:parent.width
                id: priorityBox
                Component.onCompleted: {
                    addItem("Top Priority")
                    addItem("Medium Priority")
                    addItem("Low Priority")
                    addItem("No Priority")
                }
            }
        }
    }

    Row {
        spacing:10
        anchors.bottom: taskEditor.bottom
        Item {
            width:(page.width/2)-20
            height:30
            PlasmaWidgets.PushButton {
                width:parent.width
                id:acceptButton
                text: "Update Task"

                onClicked: {
                    var task=tasks.data["Task:"+taskID]
                    var service = tasks.serviceForSource("Task:"+taskID)

                    if (task.name != nameEdit.text) {
                        var cg = service.operationDescription("setName")
                        cg.name=nameEdit.text
                        service.startOperationCall(cg)
                    }

                    if (task.due != dateEdit.text) {
                        var cg = service.operationDescription("setDueText")
                        cg.name=dateEdit.text
                        service.startOperationCall(cg)
                    }

                    if (task.tags != tagEdit.text.split(",")) {
                        var cg = service.operationDescription("setTags")
                        cg.tags=tagEdit.text.split(",")
                        service.startOperationCall(cg)
                    }

                    if (task.priority != (priorityBox.currentIndex+1)) {
                        var cg = service.operationDescription("setPriority")
                        cg.priority = priorityBox.currentIndex +1
                        service.startOperationCall(cg)
                    }

                    mainView.currentIndex=0
                }
            }
        }

        Item {
            width:(page.width/2)-20
            height:30
            PlasmaWidgets.PushButton {
                width:parent.width
                id:discardButton
                text: "Discard Changes"

                onClicked: mainView.currentIndex=0
            }
        }
    }
}