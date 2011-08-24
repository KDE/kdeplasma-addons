/*****************************************************************************
*   Copyright (C) 2011 by Shaun Reich <shaun.reich@kdemail.net>              *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    width: 160
    height: 200
    id: konqProfiles

    PlasmaCore.DataSource {
        id: profilesSource
        engine: "org.kde.plasma.dataengine.konqprofiles"
        onSourceAdded: connectSource(source)
        onSourceRemoved: disconnectSource(source)

        Component.onCompleted: connectedSources = sources
    }

    Component.onCompleted: {
        plasmoid.popupIcon = QIcon("konqueror");
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
    }


    Text {
        id: header
        text: "Konqueror Profiles"
        anchors { top: parent.top; left: parent.left; right: parent.right }
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaWidgets.Separator {
        id: separator
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        anchors { topMargin: 3 }
    }

    ListView {
        id: profileView
        anchors {
            top : separator.bottom
            topMargin: 10
            bottom: konqProfiles.bottom
            left: parent.left
            right: parent.right
        }

        model: PlasmaCore.DataModel {
            dataSource: profilesSource
            sourceFilter: "name:.*"
        }

        delegate: profileViewDelegate
        highlight: profileViewHighlighter
        highlightMoveDuration: 250
        highlightMoveSpeed: 1
        clip: true

        Component.onCompleted: currentIndex = -1
    }

    //we use this to compute a fixed height for the items, and also to implement
    //the said constant below (itemHeight)
    Text {
        id: heightMetric
        visible: false
        text: "Arbitrary String"
    }

    property int itemHeight: heightMetric.height * 2
    Component {
        id: profileViewDelegate

        Item {
            height: itemHeight
            anchors { left: parent.left; leftMargin: 10; right: parent.right }

            Text {
                id: text
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                text: model.prettyName
            }

            MouseArea {
                height: itemHeight
                anchors { left: parent.left; right: parent.right }
                hoverEnabled: true

                onClicked: {
                    var service = profilesSource.serviceForSource(model.name)
                    var operation = service.operationDescription("open")
                    var job = service.startOperationCall(operation)
                }

                onEntered: {
                    profileView.currentIndex = index
                    profileView.highlightItem.opacity = 1
                }

                onExited: {
                    profileView.highlightItem.opacity = 0
                }
            }
        }
    }

    Component {
        id: profileViewHighlighter

        PlasmaCore.FrameSvgItem {
            width: konqProfiles.width
            imagePath: "widgets/viewitem"
            prefix: "hover"
            opacity: 0
            Behavior on opacity { NumberAnimation { duration: 250 } }
        }
    }
}
