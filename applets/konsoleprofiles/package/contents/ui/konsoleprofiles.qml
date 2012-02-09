/*****************************************************************************
*   Copyright (C) 2011, 2012 by Shaun Reich <shaun.reich@kdemail.net>        *
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

import QtQuick 1.1
import org.kde.qtextracomponents 0.1

import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
   id: konsoleProfiles

    property int minimumWidth: 200
    property int minimumHeight: 300

    PlasmaCore.DataSource {
        id: profilesSource
        engine: "org.kde.konsoleprofiles"
        onSourceAdded: connectSource(source)
        onSourceRemoved: disconnectSource(source)

        Component.onCompleted: connectedSources = sources
    }

    PlasmaCore.DataModel {
        id: profilesModel
        dataSource: profilesSource
    }

    Component.onCompleted: {
        plasmoid.popupIcon = "utilities-terminal";
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
    }


   PlasmaCore.Svg {
       id: lineSvg
       imagePath: "widgets/line"
    }



        Row {
            id: headerRow
            anchors { left: parent.left; right: parent.right}

//            height: parent.height
 //           width: parent.width

            QIconItem {
                icon: QIcon("utilities-terminal")
                width: 32
                height: 32
            }

            PlasmaComponents.Label {
                id: header
                text: i18n("Konsole Profiles")
                anchors { top: parent.top;}// left: parent.left; right: parent.right }
                horizontalAlignment: Text.AlignHCenter
            }
        }

            PlasmaCore.SvgItem {
                id: separator

//                anchors { left: parent.left; right: parent.right } //top: header.bottom;  }
//                anchors { topMargin: 3 }
anchors { left: headerRow.left; right: headerRow.right; top: headerRow.bottom }

                svg: lineSvg
                elementId: "horizontal-line"
                height: lineSvg.elementSize("horizontal-line").height
            }


        //we use this to compute a fixed height for the items, and also to implement
        //the said constant below (itemHeight)
        Text {
            id: textMetric
            visible: false
            // i think this should indeed technically be translated, even though we won't ever use it, just
            // its height/width
            text: i18n("Arbitrary String Which Says The Dictionary Type")
        }

//            anchors { top: parent.top; bottom: parent.bottom }
//            width: parent.width //- scrollBar.width
//            height: parent.height
// //FIXME: wtf? work scrollbar, work damn you
// contentHeight: 20 * view.count
//            clip: true
//
            ListView {
                id: view

//                anchors { left: parent.left; right: scrollBar.left; bottom: parent.bottom; top: parent.top }
                anchors { left: parent.left; right: scrollBar.left; bottom: parent.bottom; top: separator.bottom }
//anchors.fill: parent
                anchors.topMargin: 10

                model: profilesModel
                spacing: 15

                delegate: Item {
                    id: listdelegate
                    height: textMetric.paintedHeight * 2
                    anchors { left: parent.left; leftMargin: 10; right: parent.right; rightMargin: 10 }

                    Text {
                        id: profileText
                        anchors.fill: parent
                        //anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                        text: model.prettyName
                    }

                    MouseArea {
                        height: parent.height + 15
                        anchors { left: parent.left; right: parent.right;}
                        hoverEnabled: true

                        onClicked: {
                            var service = profilesSource.serviceForSource(model["DataEngineSource"])
                            var operation = service.operationDescription("open")
                            var job = service.startOperationCall(operation)
                        }

                        onEntered: {
                            view.currentIndex = index
                            view.highlightItem.opacity = 1
                        }

                        onExited: {
                            view.highlightItem.opacity = 0
                        }
                    }
                }

                highlight: PlasmaComponents.Highlight {
                    anchors { left: parent.left; right: parent.right; leftMargin: 10; rightMargin: 10 }
                    height: textMetric.paintedHeight
                    hover: true;
                }

                highlightMoveDuration: 250
                highlightMoveSpeed: 1


            }
             
        PlasmaComponents.ScrollBar {
            id: scrollBar

          //  anchors { right: parent.right }
//          anchors { bottom:parent.bottom; top: parent.top; right: parent.right}
anchors { bottom: parent.bottom; top: separator.top; right: parent.right }

            orientation: Qt.Vertical
       //     stepSize: 30 // textBrowser.lineCount / 4
        //    scrollButtonInterval: 30 //textBrowser.lineCount / 4

            flickableItem: view
        }




//    property int itemHeight: heightMetric.height * 2
}
