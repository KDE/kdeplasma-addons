/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

import org.kde.draganddrop 2.0 as DragDrop

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import plasma.applet.org.kde.plasma.mediaframe

PlasmoidItem {
    id: main

    MediaFrame {
        id: items
        random: plasmoid.configuration.randomize
    }

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground
    Plasmoid.icon: activeSource.length > 0 ? activeSource : "settings-configure"

    width: Kirigami.Units.gridUnit * 23
    height: Kirigami.Units.gridUnit * 15

    property string activeSource: ""
    property string transitionSource: ""

    readonly property bool pause: fullRepresentationItem?.overlayMouseArea?.containsMouse ?? false

    readonly property int itemCount: (items.count + items.futureLength)
    readonly property bool hasItems: ((itemCount > 0) || (items.futureLength > 0))
    readonly property bool isTransitioning: fullRepresentationItem?.faderAnimation.running ?? false

    onActiveSourceChanged: {
        items.watch(activeSource)
    }

    onHasItemsChanged: {
        if(hasItems) {
            if(activeSource == "")
                nextItem()
        }
    }

    onExternalData: (mimetype, data) => {
        var type = items.isDir(data) ? "folder" : "file";
        var item = {
            "path": data,
            "type": type
        };

        addItem(item);
    }

    function loadPathList() {
        var list = plasmoid.configuration.pathList
        items.clear()
        for(var i in list) {
            var item = JSON.parse(list[i])
            items.add(item.path,true)
        }
    }

    Component.onCompleted: {
        loadPathList()

        if (items.random)
            nextItem()
    }

    Connections {
        target: plasmoid.configuration
        function onPathListChanged() {
            loadPathList()
        }
    }

    function addItem(item) {

        if(items.isAdded(item.path)) {
            return
        }
        // work-around for QTBUG-67773:
        // C++ object property of type QVariant(QStringList) is not updated on changes from QML
        // so explicitly create a deep JSValue copy, modify that and then set it back to overwrite the old
        var updatedList = plasmoid.configuration.pathList.slice();
        updatedList.push(JSON.stringify(item));
        plasmoid.configuration.pathList = updatedList;
    }

    function nextItem() {

        if(!hasItems) {
            return
        }

        var active = activeSource

        // Only record history if we have more than one item
        if(itemCount > 1)
            items.pushHistory(active)

        if(items.futureLength > 0) {
            setActiveSource(items.popFuture())
        } else {
            //setLoading()
            items.get(function(filePath){
                setActiveSource(filePath)
                //unsetLoading()
            },function(errorMessage){
                //unsetLoading()
            })
        }
    }

    function previousItem() {
        var active = activeSource
        items.pushFuture(active)
        var filePath = items.popHistory()
        setActiveSource(filePath)
    }

    function setActiveSource(source) {
        if (itemCount > 1) {
            transitionSource = source
        } else {
            transitionSource = source
            activeSource = source
        }

        if (fullRepresentationItem) {
            fullRepresentationItem.faderAnimation.restart()
        }
    }

    Connections {
        target: items

        function onItemChanged(path) {
            activeSource = ""
            setActiveSource(path)
        }

    }

    Timer {
        id: nextTimer
        interval: (plasmoid.configuration.interval*1000)
        repeat: true
        running: hasItems && !pause
        onTriggered: nextItem()
    }

    fullRepresentation: Item {
        property alias faderAnimation: faderAnimation
        property alias overlayMouseArea: overlayMouseArea

        property Image frontImage: image1
        property Image bufferImage: image2

        Item {
            id: itemView
            anchors.fill: parent

            Item {
                id: imageView
                visible: hasItems
                anchors.fill: parent

                // This timer prevents reloading the image too often when resizing,
                // to minimize excessively re-reading the file on disk
                Timer {
                    id: imageReloadTimer
                    interval: 250
                    running: false
                    onTriggered: {
                        image1.sourceSize.width = width
                        image1.sourceSize.height = height
                    }
                }

                Image {
                    id: image2

                    anchors.fill: parent
                    z: image2 === frontImage
                    fillMode: plasmoid.configuration.fillMode

                    opacity: 0

                    cache: false

                    asynchronous: true
                    autoTransform: true
                }

                Image {
                    id: image1

                    anchors.fill: parent
                    z: image2 === frontImage
                    fillMode: plasmoid.configuration.fillMode

                    cache: false

                    asynchronous: true
                    autoTransform: true

                    onWidthChanged: imageReloadTimer.restart()
                    onHeightChanged: imageReloadTimer.restart()

                    sourceSize.width: width
                    sourceSize.height: height

                    HoverHandler {
                        enabled: Plasmoid.configuration.leftClickOpenImage
                        cursorShape: Qt.PointingHandCursor
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        enabled: Plasmoid.configuration.leftClickOpenImage
                        onTapped: Qt.openUrlExternally(activeSource)
                    }
                }
            }
        }

        SequentialAnimation {
            id: faderAnimation

            ScriptAction {
                script: {
                    frontImage.source = activeSource
                    bufferImage.source = transitionSource
                    if (image1 === frontImage) {
                        frontImage = image2
                        bufferImage = image1
                    } else {
                        frontImage = image1
                        bufferImage = image2
                    }

                }
            }
            ParallelAnimation {
                OpacityAnimator { target: frontImage; from: 1; to: 0; duration: Kirigami.Units.veryLongDuration }
                OpacityAnimator { target: bufferImage; from: 0; to: 1; duration: Kirigami.Units.veryLongDuration }
            }
            ScriptAction {
                script: {
                    // Copy the transitionSource
                    var ts = transitionSource
                    activeSource = ts
                    bufferImage.source = activeSource
                }
            }
        }

        DragDrop.DropArea {
            id: dropArea
            anchors.fill: parent

            onDrop: {
                var mimeData = event.mimeData
                if (mimeData.hasUrls) {
                    var urls = mimeData.urls
                    for (var i = 0, j = urls.length; i < j; ++i) {
                        var url = urls[i]
                        var type = items.isDir(url) ? "folder" : "file"
                        var item = { "path":url, "type":type }
                        addItem(item)
                    }
                }
                event.accept(Qt.CopyAction)
            }
        }

        Item {
            id: overlay

            anchors.fill: parent

            visible: hasItems
            opacity: overlayMouseArea.containsMouse ? 1 : 0

            Behavior on opacity {
                NumberAnimation {}
            }

            PlasmaComponents3.Button {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                enabled: (items.historyLength > 0) && !isTransitioning
                visible: main.itemCount > 1
                icon.name: "arrow-left"
                onClicked: {
                    nextTimer.stop()
                    previousItem()
                }
            }

            PlasmaComponents3.Button {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                enabled: hasItems && !isTransitioning
                visible: main.itemCount > 1
                icon.name: "arrow-right"
                onClicked: {
                    nextTimer.stop()
                    nextItem()
                }
            }

            Row {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: Kirigami.Units.smallSpacing

                PlasmaComponents3.Button {
                    icon.name: "document-preview"
                    onClicked: Qt.openUrlExternally(main.activeSource)
                }
            }

            // BUG TODO Fix overlay so _all_ mouse events reach lower components
            MouseArea {
                id: overlayMouseArea

                anchors.fill: parent
                hoverEnabled: true

                propagateComposedEvents: true

                onPressed: mouse => mouse.accepted = false;
                onDoubleClicked: mouse => mouse.accepted = false;
            }

        }

        PlasmaExtras.PlaceholderMessage {
            id: configNeededPlaceholder
            property bool fitsInWidget: implicitWidth <= parent.width && implicitHeight <= parent.height
            anchors.centerIn: parent
            visible: !hasItems && fitsInWidget
            iconName: "viewimage-symbolic"
            text: i18nc("@info placeholdermessage if no media loaded", "No media selected")
            helpfulAction: Kirigami.Action {

                icon.name: "insert-image-symbolic"
                text: i18nc("@action:button opens settings dialog", "Choose media…")
                onTriggered: Plasmoid.internalAction("configure").trigger();
            }
        }

        PlasmaComponents3.Button {
            anchors.centerIn: parent

            visible: !hasItems && !configNeededPlaceholder.fitsInWidget
            icon.name: "insert-image-symbolic"
            text: i18nc("@action:button opens settings dialog", "Choose media…")
            onClicked: {
                Plasmoid.internalAction("configure").trigger();
            }
        }
    }
}
