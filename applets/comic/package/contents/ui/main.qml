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

    property string currentComicId

    property bool showErrorPicture: comicApplet.showErrorPicture
    property bool middleClick: comicApplet.middleClick
    property int checkNewComicStripsIntervall:30
    property int tabBarButtonStyle: comicApplet.tabBarButtonStyle
    property bool showTabBarIcon: (tabBarButtonStyle & 0x1)
    property bool showTabBarText: (tabBarButtonStyle & 0x2)

    ComicData {
        id: comicData
    }

    width: 500; height: 300;

    PlasmaCore.DataSource {
        property string oldSource
        id: providerSource
        engine: "comic"
        interval: 0
        onSourceAdded: {
            console.log("onSourceAdded = " + source);
            oldSource = source;
            connectSource(source);
        }

        onSourceRemoved: {
            console.log("onSourceRemoved = " + source);
            disconnectSource(source);
        }

        //connectedSources:sources
        onDataChanged: {
            console.log("onDataChanged = " + oldSource);

            if (!oldSource) {

            }
            else {
                console.log("processing comicData=" + oldSource);
                createComicData();
            }
        }
    }

    PlasmaCore.Theme {
        id: theme
    }

    ComicTabBar {
        id:comicTabbar
        tabModel: comicApplet.comicsModel
        showIcon: showTabBarIcon
        showText: showTabBarText
        onCurrentTabChanged: {
            busyIndicator.visible = true;
            currentComicId = key;

            //initial request will always fetch the last strips
            updateComic("");
        }
        visible: (comicApplet.comicsModel.count > 0) ? true : false
    }

    Text {
        id: topInfo
        visible: (comicApplet.showComicAuthor || comicApplet.showComicTitle)
        anchors {
            top: comicTabbar.bottom
            left: parent.left
            right: parent.right
        }
        horizontalAlignment: Text.AlignHCenter
        text: ""
    }

    QImageItem {
        id: comicImage
        width: parent.width
        height: {
            var topInfoHeight = comicTabbar.height + ((topInfo.visible) ? topInfo.height : 0);
            var bottomInfoHeight = (bottomInfo.visible) ? bottomInfo.height : 0;
            
            parent.height - topInfoHeight - bottomInfoHeight;
        }
        anchors { 
            top: (topInfo.visible) ? topInfo.bottom : comicTabbar.bottom
            topMargin: 3
        }
        smooth: true
        opacity: busyIndicator.visible ? 0.3 : 1.0
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
            top: comicImage.bottom
            left: parent.left
            right: parent.right
        }

        Text {
            id: comicIdentifier
            visible: comicApplet.showComicIdentifier
            anchors {
                left: parent.left
            }
        }

        Text {
            id:comicUrl
            visible: comicApplet.showComicUrl
            anchors {
                right: parent.right
            }
        }
    }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: comicImage

        ButtonBar {
            id: buttonBar
            visible: (comicApplet.arrowsOnHover && mouseArea.containsMouse)
            onPrevClicked: {
                console.log("Previous clicked");
                busyIndicator.visible = true;
                updateComic(comicData.prev);
            }
            onNextClicked: {
                console.log("Next clicked");
                busyIndicator.visible = true;
                updateComic(comicData.next);
            }
            onZoomClicked: {
                console.log("Zoom clicked width = " + comicData.image.nativeWidth);
                
                fullSizeDialog.fullImage.image = comicData.image.image;
                //fullSizeDialog.fullImage.imageWidth = 100;//comicData.image.nativeWidth;
                //fullSizeDialog.fullImage.imageHeight = comicData.image.nativeHeight;
                fullSizeDialog.visible = true;
            }
        }
    }
    
    function createComicData() {
        var data = providerSource.data[providerSource.oldSource];
        
        if (typeof data === "undefined") {
            console.log("comicData is undefined");
            return;
        }

        comicData.setData(data);

        comicData.id = currentComicId;
        console.log("Comic Id = " + comicData.id);

        comicImage.image = comicData.image.image;
        busyIndicator.visible = false;

        updateComicInfoView();
    }

    function updateComicInfoView() {
        //update comic info
        topInfo.text = "";
        if (comicApplet.showComicAuthor && comicData.author != undefined) {
            topInfo.text = comicData.author;
        }

        if (comicApplet.showComicTitle && comicData.title != undefined) {
            if (topInfo.text != "") {
                topInfo.text += ": ";
            }
            topInfo.text += comicData.title;
        }

        if (comicApplet.showComicUrl && comicData.websiteUrl != undefined) {
            comicUrl.text = comicData.websiteUrl;

            var regExp = /^(?:([^:\/?#]+):)?(?:\/\/((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?))?((((?:[^?#\/]*\/)*)([^?#]*))(?:\?([^#]*))?(?:#(.*))?)/;
            var r = regExp.exec(comicData.websiteUrl)

            comicUrl.text = r[6];
        }
    }

    function updateComic(identifierSuffix) {
        var comic = currentComicId + ":" + identifierSuffix;
        console.log("retrieve data for " + comic);
        providerSource.disconnectSource(providerSource.oldSource);
        providerSource.connectSource(comic);
    }
}
