/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import QtNetwork as QtNetwork

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

    signal showNextNewStripRequested

    readonly property bool showComicAuthor: Plasmoid.configuration.showComicAuthor
    readonly property bool showComicTitle: Plasmoid.configuration.showComicTitle
    readonly property bool middleClick: Plasmoid.configuration.middleClick

    readonly property Comic.comicData comicData: Plasmoid.comicData

    property bool configNeededPlaceholderVisible: false
    property bool currentTabHighlighted: false

    function updateComic(comicId : string) : void {
        Plasmoid.updateComic(comicId)
    }

    Binding {
        target: Plasmoid
        property: "configurationRequired"
        value: Plasmoid.configuration.tabIdentifier.length === 0
            && !mainWindow.configNeededPlaceholderVisible
            && mainWindow.height >= mainWindow.switchHeight
            && mainWindow.width >= mainWindow.switchWidth

    }

    Timer {
        id: retryTimer
        running: Plasmoid.configuration.tabIdentifier.length > 0 && mainWindow.comicData.isError
        interval: 30 * 60 * 1000 // every 30 minutes
        onTriggered: mainWindow.updateComic(mainWindow.comicData.current)
    }

    Connections {
        target: QtNetwork.NetworkInformation
        function onReachabilityChanged() {
            if (QtNetwork.NetworkInformation.reachability == QtNetwork.NetworkInformation.Reachability.Online && mainWindow.comicData.isError) {
                mainWindow.updateComic(mainWindow.comicData.current)
            }
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: enabledComicsModel
        sourceModel: Plasmoid.availableComicsModel
        filterRowCallback: function(source_row, source_parent) {
            return sourceModel.index(source_row, 0, source_parent).data(KItemModels.KRoleNames.role("enabled"));
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready && mainWindow.currentTabHighlighted
            visible: Plasmoid.configuration.checkNewComicStripsIntervall
            text: i18nc("@action comic strip", "&Next Tab with a New Strip") // qmllint disable unqualified
            icon.name: "go-next-view"
            shortcut: StandardKey.New
            onTriggered: mainWindow.showNextNewStripRequested()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready && mainWindow.comicData.hasFirst
            text: i18nc("@action", "Jump to &First Strip") // qmllint disable unqualified
            icon.name: "go-first"
            onTriggered: mainWindow.updateComic(mainWindow.comicData.first)
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "Jump to &Current Strip") // qmllint disable unqualified
            icon.name: "go-last"
            onTriggered: mainWindow.updateComic("")
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "Jump to Strip…") // qmllint disable unqualified
            icon.name: "go-jump"
            onTriggered: jumpDialog.open()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "Visit the Website") // qmllint disable unqualified
            onTriggered: mainWindow.comicData.launchWebsite()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready && mainWindow.comicData.shopUrl.toString() !== ""
            text: i18nc("@action", "Visit the Shop &Website") // qmllint disable unqualified
            onTriggered: mainWindow.comicData.launchShop()
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@action", "&Save Comic As…") // qmllint disable unqualified
            icon.name: "document-save-as"
            onTriggered: {
                saveDialog.checkCurrentFolder()
                saveDialog.open()
            }
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@option:check Context menu of comic image", "&Actual Size") // qmllint disable unqualified
            icon.name: "zoom-original"
            checkable: true
            checked: Plasmoid.showActualSize ?? false
            onTriggered: Plasmoid.showActualSize = this.checked
        },
        PlasmaCore.Action {
            enabled: mainWindow.comicData.id != "" && mainWindow.comicData.ready
            text: i18nc("@option:check Context menu of comic image", "Store Current &Position") // qmllint disable unqualified
            icon.name: "go-home"
            checkable: true
            checked: mainWindow.comicData.storePosition ?? false
            onTriggered: Plasmoid.storePosition(this.checked)
        }
    ]

    fullRepresentation:  Item {
        id: widgetFullRepresentation

        anchors.fill: parent
        property alias comicTabbar: comicTabbar
        property alias configNeededPlaceholderVisible: configNeededPlaceholder.visible

        function showNextNewStrip() {
            var firstHighlightedButtonIndex = undefined;

            for (var i = 0; i < comicTabbar.count; ++i) {
                var button = comicTabbar.itemAt(i) as ComicTab;
                if (button.plugin !== undefined && button.highlight == true) {
                    //plugin is ordered
                    if (button.plugin > comicTabbar.currentComicTab.plugin) {
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

        Connections {
            target: mainWindow
            function onShowNextNewStripRequested() {
                widgetFullRepresentation.showNextNewStrip()
            }
        }

        Binding {
            mainWindow.configNeededPlaceholderVisible: widgetFullRepresentation.configNeededPlaceholderVisible
            mainWindow.currentTabHighlighted: comicTabbar.currentComicTab?.highlight ?? false
        }

        ColumnLayout {
            anchors.fill: parent

            PlasmaComponents3.TabBar {
                id: comicTabbar

                Layout.fillWidth: true

                readonly property ComicTab currentComicTab: comicTabbar.currentItem as ComicTab

                visible: Plasmoid.configuration.tabIdentifier.length > 1

                onCurrentComicTabChanged: {
                    if (comicTabbar.currentComicTab && comicTabbar.currentComicTab.plugin !== "") {
                        Plasmoid.tabChanged(comicTabbar.currentComicTab.plugin);
                    }
                }

                Repeater {
                    model: enabledComicsModel
                    onItemAdded: (index, item) => {
                        if ((item as ComicTab).plugin === Plasmoid.configuration.comic) {
                            comicTabbar.setCurrentIndex(index)
                        } else if (count === 1) {
                            comicTabbar.setCurrentIndex(0)
                            comicTabbar.currentIndexChanged()
                        }
                    }
                    delegate: ComicTab { }
                }

                component ComicTab: PlasmaComponents3.TabButton {
                    id: tabButton

                    required property bool highlight
                    required property string plugin
                    required property string decoration
                    required property var model

                    onToggled: {
                        Plasmoid.configuration.comic = plugin;
                        Plasmoid.configuration.writeConfig()
                    }

                    text: model.display // can't be required as it conflicts with AbstractButton
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
                        tempTop += ( ( (mainWindow.comicData.stripTitle.length > 0) && (mainWindow.comicData.title.length > 0) ) ? " - " : "" ) + (mainWindow.comicData.stripTitle ?? "");
                    }

                    if ( mainWindow.showComicAuthor && mainWindow.comicData.author.length > 0 ) {
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
                onUpdateComicRequested: comicId =>  mainWindow.updateComic(comicId)
            }

            ComicBottomInfo {
                id:bottomInfo
                Layout.fillWidth: true

                comicData: mainWindow.comicData
                showUrl: Plasmoid.configuration.showComicUrl
                showIdentifier: Plasmoid.configuration.showComicIdentifier
                onJumpClicked: jumpDialog.open()
            }

            PlasmaExtras.PlaceholderMessage {
                id: configNeededPlaceholder
                property bool fitsInWidget: implicitWidth <= parent.width && implicitHeight <= parent.height
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                visible: Plasmoid.configuration.tabIdentifier.length === 0 && fitsInWidget
                iconName: "folder-comic-symbolic"
                text: i18nc("@info placeholdermessage if no comics loaded", "No comics configured") // qmllint disable unqualified
                helpfulAction: Kirigami.Action {
                    icon.name: "configure"
                    text: i18nc("@action:button helpfulAction opens settings dialog", "Choose comic…") // qmllint disable unqualified
                    onTriggered: Plasmoid.internalAction("configure").trigger();
                }
            }
        }
    }

    FileDialog {
        id: saveDialog
        fileMode: FileDialog.SaveFile
        defaultSuffix: "png"
        currentFile: i18nc("@other filename pattern %1 path %2 comic (provider) name, %3 image id", "%1/%2 - %3.png", currentFolder, mainWindow.comicData.title, mainWindow.comicData.currentReadable)  // qmllint disable unqualified

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
            mainWindow.comicData.saveImage(selectedFile)
            Plasmoid.configuration.savingDir = currentFolder
            Plasmoid.configuration.writeConfig()
        }
    }

    Comic.JumpDialog {
        id: jumpDialog
        comicData: mainWindow.comicData

        onAccepted: identifier => mainWindow.updateComic(identifier)
    }
}
