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

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: mainWindow

    property int implicitWidth: 600
    property int implicitHeight: 250

    width: minimumWidth
    height: minimumHeight

    property int minimumWidth: theme.mSize(theme.defaultFont).width * 35
    property int minimumHeight: theme.mSize(theme.defaultFont).height * 12
    property bool showComicAuthor: plasmoid.nativeInterface.showComicAuthor
    property bool showComicTitle: plasmoid.nativeInterface.showComicTitle
    property bool showErrorPicture: plasmoid.nativeInterface.showErrorPicture
    property bool middleClick: plasmoid.nativeInterface.middleClick

    Connections {
        target: plasmoid.nativeInterface

        onComicsModelChanged: {
            comicTabbar.currentTab = comicTabbar.layout.children[1];
        }

        onTabHighlightRequest: {
            for (var i = 0; i < comicTabbar.layout.children.length; ++i) {
                var button = comicTabbar.layout.children[i];

                if (button.key !== undefined && button.key == id) {
                    button.highlighted = highlight;
                }
            }
        }

        onShowNextNewStrip: {
            var firstButton = undefined;

            for (var i = 0; i < comicTabbar.layout.children.length; ++i) {
                var button = comicTabbar.layout.children[i];
                if (button.key !== undefined && button.highlighted == true) {
                    //key is ordered
                    if (button.key > comicTabbar.currentTab.key) {
                        comicTabbar.currentTab = button;
                        return;
                    } else if (firstButton === undefined){
                        firstButton = button;
                    }
                }
            }

            if (firstButton !== undefined) {
                comicTabbar.currentTab = firstButton;
            }
        }
    }

    PlasmaCore.Svg {
        id: arrowsSvg
        imagePath: "widgets/arrows"
    }

    PlasmaComponents.TabBar{
        id: comicTabbar

        anchors {
            left: parent.left
            right: parent.right
        }

        visible: (plasmoid.nativeInterface.comicsModel.count > 1)

        onCurrentTabChanged: {
            console.log("onCurrentTabChanged:" + comicTabbar.currentTab.key);
            plasmoid.nativeInterface.tabChanged(comicTabbar.currentTab.key);
        }

        Repeater {
            model: plasmoid.nativeInterface.comicsModel
            delegate:  PlasmaComponents.TabButton {
                id: tabButton

                property string key: model.key
                property bool highlighted: model.highlight

                text: model.title
                iconSource: model.icon

                Rectangle {
                    id: highlightMask

                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                    }

                    width: Math.max(units.iconSizes.small, tabButton.height)
                    height: Math.max(units.iconSizes.small, tabButton.height)

                    color: "white"
                    opacity: model.highlight ? 0 : 0.5
                }
            }
        }
    }

    PlasmaComponents.Label {
        id: topInfo

        anchors {
            top: comicTabbar.visible ? comicTabbar.bottom : mainWindow.top
            left: mainWindow.left
            right: mainWindow.right
        }

        visible: (topInfo.text.length > 0)
        horizontalAlignment: Text.AlignHCenter
        text: (showComicAuthor || showComicTitle) ? getTopInfo() : ""

        function getTopInfo() {
            var tempTop = "";

            if ( showComicTitle ) {
                tempTop = plasmoid.nativeInterface.comicData.title;
                tempTop += ( ( (plasmoid.nativeInterface.comicData.stripTitle.length > 0) && (plasmoid.nativeInterface.comicData.title.length > 0) ) ? " - " : "" ) + plasmoid.nativeInterface.comicData.stripTitle;
            }

            if ( showComicAuthor &&
                (plasmoid.nativeInterface.comicData.author != undefined || plasmoid.nativeInterface.comicData.author.length > 0) ) {
                tempTop = ( tempTop.length > 0 ? plasmoid.nativeInterface.comicData.author + ": " + tempTop : plasmoid.nativeInterface.comicData.author );
            }

            return tempTop;
        }
    }

    ComicCentralView {
        id: centerLayout

        anchors {
            left: mainWindow.left
            right: mainWindow.right
            bottom: (bottomInfo.visible) ? bottomInfo.top : mainWindow.bottom
            top: (topInfo.visible) ? topInfo.bottom : (comicTabbar.visible ? comicTabbar.bottom : mainWindow.top)
            topMargin: (comicTabbar.visible) ? 3 : 0
        }

        comicData: plasmoid.nativeInterface.comicData
    }

    ComicBottomInfo {
        id:bottomInfo

        anchors {
            left: mainWindow.left
            right: mainWindow.right
            bottom: mainWindow.bottom
        }

        comicData: plasmoid.nativeInterface.comicData
        showUrl: plasmoid.nativeInterface.showComicUrl
        showIdentifier: plasmoid.nativeInterface.showComicIdentifier
    }

    PlasmaComponents.BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: visible
        visible: false
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
}
