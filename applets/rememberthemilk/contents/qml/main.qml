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
    id: page
    preferredSize: "350x500" //FIXME: Doesn't get noticed

    Component.onCompleted: {
        var authService = sources.serviceForSource("Auth")

        var token = plasmoid.readConfig("token")
        if (token=="")
            mainView.currentIndex=2
        else {
            var cg = authService.operationDescription("AuthWithToken")
            cg.token=token
            authService.startOperationCall(cg)
            plasmoid.busy = true
        }
    }

//---------------------------------------------------------------Data-Sources-------------------------------------
    Item {
        PlasmaCore.DataSource {
            id: sources
            engine: "rtm"
            connectedSources: ["Lists","Auth"]
            interval: 30000

            onNewData: {
                switch (sourceName) {
                case "Auth":
                    if ( data["ValidToken"] == true ) {
                        if (mainView.currentIndex==2)
                            mainView.currentIndex=0
                        plasmoid.writeConfig("token", data["Token"])
                    }

                case "Lists":
                    for (i in data)
                        lists.connectSource("List:"+i)
                }
            }
        }

        PlasmaCore.DataSource {
            id: lists
            engine: "rtm"
            interval: 30000

            onSourceDisconnected: taskList.removeList(source.slice(5))

            onNewData: taskList.updateLists(data)

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

//---------------------------------------------------------------Views-------------------------------------
    PlasmaWidgets.TabBar {
        id : mainView
        anchors.fill : page
        tabBarShown: false

        TaskList {
            id : taskList
        }
        TaskEditor {
            id : taskEditor
        }
        AuthRequiredView {
            id : authRequiredView
        }
    }
}