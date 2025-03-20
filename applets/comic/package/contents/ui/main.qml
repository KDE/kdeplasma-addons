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
    readonly property bool showComicAuthor: plasmoid.showComicAuthor
    readonly property bool showComicTitle: plasmoid.showComicTitle
    readonly property bool showErrorPicture: plasmoid.showErrorPicture
    readonly property bool middleClick: plasmoid.middleClick

    Connections {
        target: plasmoid

        function onComicsModelChanged() {
            comicTabbar.setCurrentIndex(0);
        }

        function onTabHighlightRequest(id, highlight) {
            for (var i = 0; i < comicTabbar.count; ++i) {
                var button = comicTabbar.itemAt(i);

                if (button.key !== undefined && button.key == id) {
                    button.highlighted = highlight;
                }
            }
        }

        function onShowNextNewStrip() {
            var firstHighlightedButtonIndex = undefined;

            for (var i = 0; i < comicTabbar.count; ++i) {
                var button = comicTabbar.itemAt(i);
                if (button.key !== undefined && button.highlighted == true) {
                    //key is ordered
                    if (button.key > comicTabbar.currentItem.key) {
                        comicTabbar.setCurrentIndex(i);
                        return;
                    } else if (firstHighlightedButtonIndex === undefined){
                        firstHighlightedButtonIndex = button;
                    }
                }
            }

            if (firstHighlightedButtonIndex !== undefined) {
                comicTabbar.setCurrentIndex(firstHighlightedButtonIndex);
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        PlasmaComponents3.TabBar {
            id: comicTabbar

            Layout.fillWidth: true

            visible: plasmoid.tabIdentifiers.length > 1

            onCurrentIndexChanged: {
                console.log("onCurrentTabChanged:" + comicTabbar.currentItem.key);
                plasmoid.tabChanged(comicTabbar.currentItem.key);
            }

            Repeater {
                model: plasmoid.comicsModel
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

            visible: (topInfo.text.length > 0)
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: (showComicAuthor || showComicTitle) ? getTopInfo() : ""
            textFormat: Text.PlainText
            elide: Text.ElideRight

            function getTopInfo() {
                var tempTop = "";

                if ( showComicTitle ) {
                    tempTop = plasmoid.comicData.title;
                    tempTop += ( ( (plasmoid.comicData.stripTitle.length > 0) && (plasmoid.comicData.title.length > 0) ) ? " - " : "" ) + plasmoid.comicData.stripTitle;
                }

                if ( showComicAuthor &&
                    (plasmoid.comicData.author != undefined || plasmoid.comicData.author.length > 0) ) {
                    tempTop = ( tempTop.length > 0 ? plasmoid.comicData.author + ": " + tempTop : plasmoid.comicData.author );
                }

                return tempTop;
            }
        }

        ComicCentralView {
            id: centerLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: comicTabbar.visible ? 3 : 0

            visible: plasmoid.tabIdentifiers.length > 0
            comicData: plasmoid.comicData
        }

        ComicBottomInfo {
            id:bottomInfo
            Layout.fillWidth: true

            comicData: plasmoid.comicData
            showUrl: plasmoid.showComicUrl
            showIdentifier: plasmoid.showComicIdentifier
        }


    }
}
