/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents // PC3 TabBar+TabButton need work first
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: mainWindow

    readonly property int implicitWidth: PlasmaCore.Units.gridUnit * 40
    readonly property int implicitHeight: PlasmaCore.Units.gridUnit * 15
    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground
    Plasmoid.switchWidth: {
        if (centerLayout.comicData.image) {
            return Math.max(minimumWidth, Math.min(centerLayout.comicData.image.nativeWidth * 0.6, implicitWidth));
        } else {
            return PlasmaCore.Units.gridUnit * 8;
        }
    }
    Plasmoid.switchHeight: {
        if (centerLayout.comicData.image) {
            return Math.max(minimumHeight, Math.min(centerLayout.comicData.image.nativeHeight * 0.6, implicitHeight));
        } else {
            return PlasmaCore.Units.gridUnit * 8;
        }
    }
    Plasmoid.icon: "face-laughing"

    width: implicitWidth
    height: implicitHeight

    readonly property int minimumWidth: PlasmaCore.Units.gridUnit * 8
    readonly property int minimumHeight: PlasmaCore.Units.gridUnit * 8
    readonly property bool showComicAuthor: plasmoid.nativeInterface.showComicAuthor
    readonly property bool showComicTitle: plasmoid.nativeInterface.showComicTitle
    readonly property bool showErrorPicture: plasmoid.nativeInterface.showErrorPicture
    readonly property bool middleClick: plasmoid.nativeInterface.middleClick

    Connections {
        target: plasmoid.nativeInterface

        function onComicsModelChanged() {
            comicTabbar.currentTab = comicTabbar.layout.children[1];
        }

        function onTabHighlightRequest(id, highlight) {
            for (var i = 0; i < comicTabbar.layout.children.length; ++i) {
                var button = comicTabbar.layout.children[i];

                if (button.key !== undefined && button.key == id) {
                    button.highlighted = highlight;
                }
            }
        }

        function onShowNextNewStrip() {
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

        visible: plasmoid.nativeInterface.tabIdentifiers.length > 1

        onCurrentTabChanged: {
            console.log("onCurrentTabChanged:" + comicTabbar.currentTab.key);
            plasmoid.nativeInterface.tabChanged(comicTabbar.currentTab.key);
        }

        Repeater {
            model: plasmoid.nativeInterface.comicsModel
            delegate:  PlasmaComponents.TabButton {
                id: tabButton

                readonly property string key: model.key
                property bool highlighted: model.highlight

                text: model.title
                iconSource: model.icon
            }
        }
    }

    PlasmaComponents3.Label {
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

        visible: plasmoid.nativeInterface.tabIdentifiers.length > 0
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
                duration: PlasmaCore.Units.veryLongDuration
                easing.type: Easing.InOutQuad
            }
        }
}
