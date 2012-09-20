/*
 * Copyright 2012  Reza Fatahilah Shah <rshah0385@kireihana.com>
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
import org.kde.qtextracomponents 0.1

Item {
    id: mainWindow
    property int minimumWidth: 150
    property int minimumHeight: 60

    property bool showComicAuthor: comicApplet.showComicAuthor
    property bool showComicTitle: comicApplet.showComicTitle

    property bool showErrorPicture: comicApplet.showErrorPicture
    property bool middleClick: comicApplet.middleClick
    property int comicsModelCount: comicApplet.comicsModel.count

    width: 500; height: 300;

    PlasmaCore.Theme {
        id: theme
    }
    
    Connections {
        target: comicApplet
        onTabHighlightRequest: {
            console.log("onTabHighlightRequest:" + index);
            comicTabbar.setTabHighlighted(index, highlight);
        }
        
        onShowNextNewStrip: {
            console.log("onshowNextNewStrip");
            var index = comicTabbar.currentButtonIndex();
            comicTabbar.setCurrentButtonIndex(comicTabbar.nextHighlightedTab(index));
        }
    }

    onComicsModelCountChanged: {
        console.log("Providers count:" + comicApplet.comicsModel.count);
        comicTabbar.setCurrentButtonIndex(0);
    }

    states: [
        State {
            name: "topInfoVisible"
            when: topInfo.visible && !bottomInfo.visible
            AnchorChanges {
                target: centerLayout
                anchors.top: topInfo.bottom
            }
        },
        State {
            name: "bottomInfoVisible"
            when: bottomInfo.visible && !topInfo.visible
            AnchorChanges {
                target: centerLayout
                anchors.bottom: bottomInfo.top
            }
        },
        State {
            name: "topBottomInfoVisible"
            when: bottomInfo.visible && topInfo.visible
            AnchorChanges {
                target: centerLayout
                anchors.top: topInfo.bottom
                anchors.bottom: bottomInfo.top
            }
        }
    ]

    transitions:
        Transition {
            AnchorAnimation {
                duration: 500
                easing.type: Easing.InOutQuad
            }
        }

    PlasmaCore.Svg {
        id: arrowsSvg
        imagePath: "widgets/arrows"
    }
    
    ComicTabBar {
        id:comicTabbar
        tabModel: comicApplet.comicsModel
        showText: (comicApplet.tabBarButtonStyle & 0x1)
        showIcon: (comicApplet.tabBarButtonStyle & 0x2)
        onCurrentTabChanged: {
            //busyIndicator.visible = true;
            comicApplet.tabChanged(newIndex);
        }
        visible: (comicsModel.count > 1)
    }

    PlasmaComponents.Label {
        id: topInfo
        visible: (topInfo.text.length > 0)
        anchors {
            top: comicTabbar.visible ? comicTabbar.bottom : mainWindow.top
            left: mainWindow.left
            right: mainWindow.right
        }
        horizontalAlignment: Text.AlignHCenter
        text: (showComicAuthor || showComicTitle) ? getTopInfo() : ""

        function getTopInfo() {
            var tempTop = "";

            if ( showComicTitle ) {
                tempTop = comicApplet.comicData.title;
                tempTop += ( ( (comicApplet.comicData.stripTitle.length > 0) && (comicApplet.comicData.title.length > 0) ) ? " - " : "" ) + comicApplet.comicData.stripTitle;
            }

            if ( showComicAuthor && 
                (comicApplet.comicData.author != undefined || comicApplet.comicData.author.length > 0) ) {
                tempTop = ( tempTop.length > 0 ? comicApplet.comicData.author + ": " + tempTop : comicApplet.comicData.author );
            }

            return tempTop;
        }
    }

    ComicCentralView {
        id: centerLayout
        comicData: comicApplet.comicData
        anchors {
            left: mainWindow.left
            right: mainWindow.right
            bottom: (bottomInfo.visible) ? bottomInfo.top : mainWindow.bottom
            top: (topInfo.visible) ? topInfo.bottom : (comicTabbar.visible ? comicTabbar.bottom : mainWindow.top)
            topMargin: (comicTabbar.visible) ? 3 : 0
        }
    }

    ComicBottomInfo {
        id:bottomInfo
        comicData: comicApplet.comicData
        showUrl: comicApplet.showComicUrl
        showIdentifier: comicApplet.showComicIdentifier

        anchors {
            left: mainWindow.left
            right: mainWindow.right
            bottom: mainWindow.bottom
        }
    }

    PlasmaComponents.BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: visible
        visible: false
    }
}
