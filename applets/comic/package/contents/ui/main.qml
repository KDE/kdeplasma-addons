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

    property bool showComicUrl: comicApplet.showComicUrl
    property bool showComicAuthor: comicApplet.showComicAuthor
    property bool showComicTitle: comicApplet.showComicTitle
    property bool showComicIdentifier: comicApplet.showComicIdentifier
    property bool showErrorPicture: comicApplet.showErrorPicture
    property bool middleClick: comicApplet.middleClick
    property int tabBarButtonStyle: comicApplet.tabBarButtonStyle
    property bool showTabBarIcon: (tabBarButtonStyle & 0x1)
    property bool showTabBarText: (tabBarButtonStyle & 0x2)
    property variant comicData: comicApplet.comicData
    property variant comicsModel: comicApplet.comicsModel
    property int comicsModelCount: comicsModel.count

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
        console.log("Providers count:" + comicsModel.count);
        console.log("Providers count:" + comicsModelCount);
    }
    
    onComicDataChanged: {
        //console.log("Comic data changed currentReadable:" + comicData.currentReadable);
        //console.log("Comic data changed host:" + comicData.websiteHost.length);
        busyIndicator.visible = false;
    }
    
    PlasmaCore.Svg {
        id: arrowsSvg
        imagePath: "widgets/arrows"
    }
    
    ComicTabBar {
        id:comicTabbar
        tabModel: comicsModel
        showIcon: showTabBarIcon
        showText: showTabBarText
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
            top: comicTabbar.visible ? comicTabbar.bottom : parent.top
            left: parent.left
            right: parent.right
        }
        horizontalAlignment: Text.AlignHCenter
        text: (showComicAuthor || showComicTitle) ? getTopInfo() : ""
    }
    
    function getTopInfo() {
        var tempTop = "";

        if ( showComicTitle ) {
            tempTop = comicData.title;
            tempTop += ( ( (comicData.stripTitle.length > 0) && (comicData.title.length > 0) ) ? " - " : "" ) + comicData.stripTitle;
        }
        
        if ( showComicAuthor && 
            (comicData.author != undefined || comicData.author.length > 0) ) {
            tempTop = ( tempTop.length > 0 ? comicData.author + ": " + tempTop : comicData.author );
        }
        
        return tempTop;
    }

    Row {
        id: centerLayout
        width: parent.width
        spacing: 2
        height: {
            var topInfoHeight = ((comicTabbar.visible) ? comicTabbar.height : 0) + ((topInfo.visible) ? topInfo.height : 0);
            var bottomInfoHeight = (bottomInfo.visible) ? bottomInfo.height : 0;
            
            parent.height - topInfoHeight - bottomInfoHeight - anchors.topMargin;
        }
        anchors { 
            top: (topInfo.visible) ? topInfo.bottom : (comicTabbar.visible ? comicTabbar.bottom : parent.top)
            topMargin: 3
        }

        ActionButton {
            id: arrowLeft
            svg: arrowsSvg
            elementId: "left-arrow"
            width: 40
            height: 40
            anchors {
                left: parent.left
                rightMargin: 2
                verticalCenter: parent.verticalCenter
            }
            visible: (!comicApplet.arrowsOnHover && (comicData.prev.length > 0))
            onClicked: {
                busyIndicator.visible = true;
                comicApplet.updateComic(comicData.prev);
            }
        }

        QImageItem {
            id: comicImage
            height: parent.height
            anchors { 
                left: arrowLeft.visible ? arrowLeft.right : parent.left
                right: arrowRight.visible ? arrowRight.left : parent.right
            }
            smooth: true
            opacity: busyIndicator.visible ? 0.3 : 1.0
            image: comicData.image
        }

        ActionButton {
            id: arrowRight
            svg: arrowsSvg
            elementId: "right-arrow"
            width: 40
            height: 40
            anchors {
                right: parent.right
                leftMargin: 2
                verticalCenter: parent.verticalCenter
            }
            visible: (!comicApplet.arrowsOnHover && (comicData.next.length > 0))
            onClicked: {
                busyIndicator.visible = true;
                comicApplet.updateComic(comicData.prev);
            }
        }
        
        MouseArea {
            id: mouseArea
            hoverEnabled: true
            anchors.fill: comicImage

            PlasmaCore.ToolTip {
                id: tooltip
                target: mouseArea
                mainText: comicData.additionalText
            }
                    
            ButtonBar {
                id: buttonBar
                visible: (comicApplet.arrowsOnHover && mouseArea.containsMouse)
                opacity: 0
                states: State {
                    name: "show"; when: (comicApplet.arrowsOnHover && mouseArea.containsMouse)
                    PropertyChanges { target: buttonBar; opacity: 1; }
                }

                transitions: Transition {
                    from: ""; to: "show"; reversible: true
                    NumberAnimation { properties: "opacity"; duration: 250; easing.type: Easing.InOutQuad }
                }

                onPrevClicked: {
                    console.log("Previous clicked");
                    busyIndicator.visible = true;
                    comicApplet.updateComic(comicData.prev);
                }
                onNextClicked: {
                    console.log("Next clicked");
                    busyIndicator.visible = true;
                    comicApplet.updateComic(comicData.next);
                }
                onZoomClicked: {
                    console.log("Zoom clicked width = " + comicData.image.nativeWidth);
                    comicApplet.showFullView();
                    //TODO try using qml
                    /*fullSizeDialog.fullImage.image = comicData.image.image;
                    //fullSizeDialog.fullImage.imageWidth = 100;//comicData.image.nativeWidth;
                    //fullSizeDialog.fullImage.imageHeight = comicData.image.nativeHeight;
                    fullSizeDialog.visible = true;*/
                }
            }
        }
    }

    PlasmaComponents.BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: visible
        visible: false
    }

    Row {
        id:bottomInfo
        anchors {
            top: centerLayout.bottom
            left: parent.left
            right: parent.right
            topMargin: (comicIdentifier.visible || comicUrl.visible) ? 2 : 0
        }

        PlasmaComponents.Label {
            id: comicIdentifier
            color: theme.textColor
            visible: (showComicIdentifier && comicIdentifier.text.length > 0)
            anchors {
                left: parent.left
                right: comicUrl.left
            }
            text: (showComicIdentifier && comicData.currentReadable != undefined) ? comicData.currentReadable : ""
            MouseArea {
                id: idLabelArea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    parent.color = theme.highlightColor
                }
                onExited: {
                    parent.color = theme.textColor
                }
                onClicked: {
                    console.log("Jump to Strip ...");
                    comicApplet.goJump();
                }
                PlasmaCore.ToolTip {
                    target: idLabelArea
                    mainText: i18n( "Jump to Strip ..." )
                }
            }
        }

        PlasmaComponents.Label {
            id:comicUrl
            color: theme.textColor
            visible: (showComicUrl && comicUrl.text.length > 0)
            anchors {
                right: parent.right
            }
            text: (showComicUrl && comicData.websiteHost.length > 0) ? comicData.websiteHost : ""
            MouseArea {
                id: idUrlLabelArea
                anchors.fill: parent
                hoverEnabled: true
                visible: comicApplet.checkAuthorization("LaunchApp")
                onEntered: {
                    parent.color = theme.highlightColor
                }
                onExited: {
                    parent.color = theme.textColor
                }
                onClicked: {
                    comicApplet.shop();
                }
                PlasmaCore.ToolTip {
                    target: idUrlLabelArea
                    mainText: i18n( "Visit the comic website" )
                }
            }
        }
    }
}
