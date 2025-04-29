/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kitemmodels as KItemModels

PlasmoidItem {
    id: mainWindow

    readonly property int implicitWidth: Kirigami.Units.gridUnit * 40
    readonly property int implicitHeight: Kirigami.Units.gridUnit * 15
    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.icon: "face-laughing"

    readonly property int minimumWidth: Kirigami.Units.gridUnit * 8
    readonly property int minimumHeight: Kirigami.Units.gridUnit * 8
    readonly property bool showComicAuthor: plasmoid.configuration.showComicAuthor
    readonly property bool showComicTitle: plasmoid.configuration.showComicTitle
    readonly property bool middleClick: plasmoid.configuration.middleClick

    KItemModels.KSortFilterProxyModel {
        id: enabledComicsModel
        sourceModel: plasmoid.availableComicsModel
        filterRowCallback: function(source_row, source_parent) {
            return sourceModel.index(source_row, 0, source_parent).data(KItemModels.KRoleNames.role("enabled"));
        }
        onModelReset: {
            mainWindow.fullRepresentationItem?.comicTabbar.setCurrentIndex(0);
            mainWindow.fullRepresentationItem?.comicTabbar.currentIndexChanged();
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready && mainWindow.fullRepresentationItem?.comicTabbar.currentItem.highlighted
            visible: Plasmoid.configuration.checkNewComicStripsIntervall
            text: i18nc("@action comic strip", "&Next Tab with a New Strip")
            icon.name: "go-next-view"
            shortcut: StandardKey.New
            onTriggered: Plasmoid.showNextNewStrip()
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready && plasmoid.comicData.hasFirst
            text: i18nc("@action", "Jump to &First Strip")
            icon.name: "go-first"
            onTriggered: Plasmoid.updateComic(plasmoid.comicData.first)
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready
            text: i18nc("@action", "Jump to &Current Strip")
            icon.name: "go-last"
            onTriggered: Plasmoid.updateComic("")
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready
            text: i18nc("@action", "Jump to Strip…")
            icon.name: "go-jump"
            onTriggered: plasmoid.goJump()
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready
            text: i18nc("@action", "Visit the Website")
            onTriggered: plasmoid.website()
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready && plasmoid.comicData.shopUrl !== ""
            text: i18nc("@action", "Visit the Shop &Website")
            onTriggered: plasmoid.shop()
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready
            text: i18nc("@action", "&Save Comic As…")
            icon.name: "document-save-as"
            onTriggered: plasmoid.saveComicAs()
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready
            text: i18nc("@option:check Context menu of comic image", "&Actual Size")
            icon.name: "zoom-original"
            checkable: true
            checked: plasmoid.showActualSize ?? false
            onTriggered: Plasmoid.showActualSize = this.checked
        },
        PlasmaCore.Action {
            enabled: plasmoid.comicData.id != "" && plasmoid.comicData.ready
            text: i18nc("@option:check Context menu of comic image", "Store Current &Position")
            icon.name: "go-home"
            checkable: true
            checked: plasmoid.comicData.storePosition ?? false
            onTriggered: Plasmoid.storePosition(this.checked)
        }
    ]

    fullRepresentation:  Item {
        anchors.fill: parent
        property alias comicTabbar: comicTabbar
        ColumnLayout {
            anchors.fill: parent

            Layout.preferredWidth: mainWindow.switchWidth
            Layout.preferredHeight: mainWindow.switchHeight
            Layout.minimumHeight: implicitHeight


            Connections {
                target: plasmoid

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

            PlasmaComponents3.TabBar {
                id: comicTabbar

                Layout.fillWidth: true

                visible: plasmoid.configuration.tabIdentifier.length > 1

                Component.onCompleted: currentIndexChanged()

                onCurrentIndexChanged: {
                    if (comicTabbar.currentItem && comicTabbar.currentItem.key != "") {
                        plasmoid.tabChanged(comicTabbar.currentItem.key);
                    }
                }

                Repeater {
                    model: enabledComicsModel
                    delegate:  PlasmaComponents3.TabButton {
                        id: tabButton

                        readonly property string key: model.plugin
                        property bool highlighted: model.highlight

                        text: model.display
                        icon.source: model.decoration
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
                        tempTop = plasmoid.comicData.title ?? "";
                        tempTop += ( ( (plasmoid.comicData.stripTitle?.length > 0) && (plasmoid.comicData.title?.length > 0) ) ? " - " : "" ) + (plasmoid.comicData.stripTitle ?? "");
                    }

                    if ( showComicAuthor && plasmoid.comicData.author?.length > 0 ) {
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

                visible: plasmoid.configuration.tabIdentifier.length > 0
                comicData: plasmoid.comicData
            }

            ComicBottomInfo {
                id:bottomInfo
                Layout.fillWidth: true

                comicData: plasmoid.comicData
                showUrl: plasmoid.configuration.showComicUrl
                showIdentifier: plasmoid.configuration.showComicIdentifier
            }

            PlasmaExtras.PlaceholderMessage {
                id: configNeededPlaceholder
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.margins: Kirigami.Units.gridUnit
                visible: plasmoid.configuration.tabIdentifier.length === 0
                iconName: "folder-comic-symbolic"
                text: i18nc("@info placeholdermessage if no comics loaded", "No comics configured")
                helpfulAction: Kirigami.Action {
                    icon.name: "configure"
                    text: i18nc("@action:button helpfulAction opens settings dialog", "Choose comic…")
                    onTriggered: Plasmoid.internalAction("configure").trigger();
                }
            }
        }
    }
}
