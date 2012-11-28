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

    width: minimumWidth
    height: minimumHeight

    property int minimumWidth: theme.defaultFont.mSize.width * 35
    property int minimumHeight: theme.defaultFont.mSize.height * 12
    property bool showComicAuthor: comicApplet.showComicAuthor
    property bool showComicTitle: comicApplet.showComicTitle
    property bool showErrorPicture: comicApplet.showErrorPicture
    property bool middleClick: comicApplet.middleClick

    Connections {
        target: comicApplet

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

    PlasmaCore.Theme {
        id: theme
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

        visible: (comicApplet.comicsModel.count > 1)

        onCurrentTabChanged: {
            console.log("onCurrentTabChanged:" + comicTabbar.currentTab.key);
            comicApplet.tabChanged(comicTabbar.currentTab.key);
        }

        Repeater {
            model: comicApplet.comicsModel
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

                    width: Math.max(theme.smallIconSize, tabButton.height)
                    height: Math.max(theme.smallIconSize, tabButton.height)

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

        anchors {
            left: mainWindow.left
            right: mainWindow.right
            bottom: (bottomInfo.visible) ? bottomInfo.top : mainWindow.bottom
            top: (topInfo.visible) ? topInfo.bottom : (comicTabbar.visible ? comicTabbar.bottom : mainWindow.top)
            topMargin: (comicTabbar.visible) ? 3 : 0
        }

        comicData: comicApplet.comicData
    }

    ComicBottomInfo {
        id:bottomInfo

        anchors {
            left: mainWindow.left
            right: mainWindow.right
            bottom: mainWindow.bottom
        }

        comicData: comicApplet.comicData
        showUrl: comicApplet.showComicUrl
        showIdentifier: comicApplet.showComicIdentifier
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
