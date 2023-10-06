/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0

PlasmoidItem {
    id: mainWindow

    readonly property int implicitWidth: Kirigami.Units.gridUnit * 40
    readonly property int implicitHeight: Kirigami.Units.gridUnit * 15
    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground
    switchWidth: {
        if (centerLayout.comicData.image) {
            return Math.max(minimumWidth, Math.min(centerLayout.comicData.image.nativeWidth * 0.6, implicitWidth));
        } else {
            return Kirigami.Units.gridUnit * 8;
        }
    }
    switchHeight: {
        if (centerLayout.comicData.image) {
            return Math.max(minimumHeight, Math.min(centerLayout.comicData.image.nativeHeight * 0.6, implicitHeight));
        } else {
            return Kirigami.Units.gridUnit * 8;
        }
    }
    Plasmoid.icon: "face-laughing"

    width: implicitWidth
    height: implicitHeight

    readonly property int minimumWidth: Kirigami.Units.gridUnit * 8
    readonly property int minimumHeight: Kirigami.Units.gridUnit * 8
    readonly property bool showComicAuthor: Plasmoid.showComicAuthor
    readonly property bool showComicTitle: Plasmoid.showComicTitle
    readonly property bool showErrorPicture: Plasmoid.showErrorPicture
    readonly property bool middleClick: Plasmoid.middleClick

    Connections {
        target: plasmoid

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

    Item {
        anchors.fill: parent
        PlasmaComponents3.TabBar {
            id: comicTabbar

            anchors {
                left: parent.left
                right: parent.right
            }

            visible: Plasmoid.tabIdentifiers.length > 1

            onCurrentIndexChanged: {
                console.log("onCurrentTabChanged:" + comicTabbar.currentItem.key);
                Plasmoid.tabChanged(comicTabbar.currentItem.key);
            }

            Repeater {
                model: Plasmoid.comicsModel
                delegate:  PlasmaComponents3.TabButton {
                    id: tabButton

                    readonly property string key: model.key
                    property bool highlighted: model.highlight

                    text: model.title
                    icon.source: model.icon
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
                    tempTop = Plasmoid.comicData.title;
                    tempTop += ( ( (Plasmoid.comicData.stripTitle.length > 0) && (Plasmoid.comicData.title.length > 0) ) ? " - " : "" ) + Plasmoid.comicData.stripTitle;
                }

                if ( showComicAuthor &&
                    (Plasmoid.comicData.author != undefined || Plasmoid.comicData.author.length > 0) ) {
                    tempTop = ( tempTop.length > 0 ? Plasmoid.comicData.author + ": " + tempTop : Plasmoid.comicData.author );
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

            visible: Plasmoid.tabIdentifiers.length > 0
            comicData: Plasmoid.comicData
        }

        ComicBottomInfo {
            id:bottomInfo

            anchors {
                left: mainWindow.left
                right: mainWindow.right
                bottom: mainWindow.bottom
            }

            comicData: Plasmoid.comicData
            showUrl: Plasmoid.showComicUrl
            showIdentifier: Plasmoid.showComicIdentifier
        }
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
                duration: Kirigami.Units.veryLongDuration
                easing.type: Easing.InOutQuad
            }
        }
}
