/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kitemmodels as KItemModels
import plasma.applet.org.kde.plasma.comic as Comic

PlasmoidItem {
    id: mainWindow

    width: Kirigami.Units.gridUnit * 30
    height: Kirigami.Units.gridUnit * 20
    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.icon: "face-laughing"

    readonly property bool showComicAuthor: plasmoid.configuration.showComicAuthor
    readonly property bool showComicTitle: plasmoid.configuration.showComicTitle
    readonly property bool middleClick: plasmoid.configuration.middleClick

    property Comic.comicData comicData: Plasmoid.comicData

    Binding {
        target: plasmoid
        property: "configurationRequired"
        value: plasmoid.configuration.tabIdentifier.length === 0
            && !mainWindow.fullRepresentationItem?.configNeededPlaceholderVisible
            && mainWindow.height >= mainWindow.switchHeight
            && mainWindow.width >= mainWindow.switchWidth

    }

    KItemModels.KSortFilterProxyModel {
        id: enabledComicsModel
        sourceModel: Plasmoid.availableComicsModel
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
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready && mainWindow.fullRepresentationItem?.comicTabbar.currentItem.highlighted
            visible: Plasmoid.configuration.checkNewComicStripsIntervall
            text: i18nc("@action comic strip", "&Next Tab with a New Strip")
            icon.name: "go-next-view"
            shortcut: StandardKey.New
            onTriggered: Plasmoid.showNextNewStrip()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready && mainWindow.comicData.hasFirst
            text: i18nc("@action", "Jump to &First Strip")
            icon.name: "go-first"
            onTriggered: Plasmoid.updateComic(mainWindow.comicData.first)
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "Jump to &Current Strip")
            icon.name: "go-last"
            onTriggered: Plasmoid.updateComic("")
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "Jump to Strip…")
            icon.name: "go-jump"
            onTriggered: plasmoid.goJump()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "Visit the Website")
            onTriggered: plasmoid.website()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready && mainWindow.comicData.shopUrl !== ""
            text: i18nc("@action", "Visit the Shop &Website")
            onTriggered: plasmoid.shop()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "&Save Comic As…")
            icon.name: "document-save-as"
            onTriggered: {
                saveDialog.checkCurrentFolder()
                saveDialog.open()
            }
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@option:check Context menu of comic image", "&Actual Size")
            icon.name: "zoom-original"
            checkable: true
            checked: plasmoid.showActualSize ?? false
            onTriggered: Plasmoid.showActualSize = this.checked
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@option:check Context menu of comic image", "Store Current &Position")
            icon.name: "go-home"
            checkable: true
            checked: mainWindow.comicData.storePosition ?? false
            onTriggered: Plasmoid.storePosition(this.checked)
        }
    ]

    fullRepresentation:  Item {
        anchors.fill: parent
        property alias comicTabbar: comicTabbar
        property alias configNeededPlaceholderVisible: configNeededPlaceholder.visible

        ColumnLayout {
            anchors.fill: parent

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
                text: (mainWindow.showComicAuthor || mainWindow.showComicTitle) ? getTopInfo() : ""
                textFormat: Text.PlainText
                elide: Text.ElideRight

                function getTopInfo() {
                    var tempTop = "";

                    if ( mainWindow.showComicTitle ) {
                        tempTop = mainWindow.comicData.title ?? "";
                        tempTop += ( ( (mainWindow.comicData.stripTitle?.length > 0) && (mainWindow.comicData.title?.length > 0) ) ? " - " : "" ) + (mainWindow.comicData.stripTitle ?? "");
                    }

                    if ( mainWindow.showComicAuthor && mainWindow.comicData.author?.length > 0 ) {
                        tempTop = ( tempTop.length > 0 ? mainWindow.comicData.author + ": " + tempTop : mainWindow.comicData.author );
                    }

                    return tempTop;
                }
            }

            ComicCentralView {
                id: centerLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: comicTabbar.visible ? 3 : 0

                visible: Plasmoid.configuration.tabIdentifier.length > 0
                comicData: mainWindow.comicData
            }

            ComicBottomInfo {
                id:bottomInfo
                Layout.fillWidth: true

                comicData: mainWindow.comicData
                showUrl: Plasmoid.configuration.showComicUrl
                showIdentifier: Plasmoid.configuration.showComicIdentifier
            }

            PlasmaExtras.PlaceholderMessage {
                id: configNeededPlaceholder
                property bool fitsInWidget: implicitWidth <= parent.width && implicitHeight <= parent.height
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                visible: Plasmoid.configuration.tabIdentifier.length === 0 && fitsInWidget
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

    FileDialog {
        id: saveDialog
        fileMode: FileDialog.SaveFile
        defaultSuffix: "png"
        currentFile: i18nc("@other filename pattern %1 path %2 comic (provider) name, %3 image id", "%1/%2 - %3.png", currentFolder, mainWindow.comicData.title, mainWindow.comicData.currentReadable)

        property list<string> paths: [
            Plasmoid.configuration.savingDir,
            StandardPaths.writableLocation(StandardPaths.PicturesLocation),
            StandardPaths.writableLocation(StandardPaths.DownloadLocation),
            StandardPaths.writableLocation(StandardPaths.HomeLocation)
        ]

        function checkCurrentFolder() {
            currentFolder = paths.find(path => path != "" && Plasmoid.urlExists(path))
        }

        onAccepted: {
            Plasmoid.saveImage(selectedFile)
            Plasmoid.configuration.savingDir = currentFolder
            Plasmoid.configuration.writeConfig()
        }
    }
}
