/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.draganddrop as DragDrop

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import plasma.applet.org.kde.plasma.mediaframe

PlasmoidItem {
    id: main

    MediaFrame {
        id: items
        random: Plasmoid.configuration.randomize

        onItemChanged: path => {
            main.activeSource = ""
            main.setActiveSource(path)
        }
    }

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground
    Plasmoid.icon: transitionSource || activeSource || "settings-configure"

    width: Kirigami.Units.gridUnit * 23
    height: Kirigami.Units.gridUnit * 15

    property string activeSource: ""
    property string transitionSource: ""

    property bool pause: false // set by fullRepresentation's overlayMouseArea
    property bool isTransitioning: false // set by fullRepresentation's faderAnimation

    readonly property int itemCount: (items.count + items.futureLength)
    readonly property bool hasItems: itemCount > 0

    signal restartAnimationRequested

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

        if (items.random) {
            nextItem()
        }
    }

    Connections {
        target: Plasmoid.configuration
        function onPathListChanged() {
            main.loadPathList()
            if ((!main.isTransitioning && !items.contains(main.activeSource))
                || main.isTransitioning && !items.contains(main.transitionSource)) {
                main.nextItem()
            }
        }
    }

    function addItem(item) {

        if(items.isAdded(item.path)) {
            return
        }
        Plasmoid.configuration.pathList.push(JSON.stringify(item))
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

        restartAnimationRequested()
    }

    Timer {
        id: nextTimer
        interval: (Plasmoid.configuration.interval*1000)
        repeat: true
        running: main.itemCount > 1 && !main.pause
        onTriggered: main.nextItem()
    }

    Kirigami.Action {
        id: previousAction
        enabled: (items.historyLength > 0) && !main.isTransitioning
        visible: main.itemCount > 1
        text: i18nc("@action:button icon-only", "Show previous image")
        icon.name: main.LayoutMirroring.enabled ? "go-previous-rtl-symbolic" : "go-previous-symbolic"
        onTriggered: {
            nextTimer.stop()
            main.previousItem()
        }
    }
    Kirigami.Action {
        id: nextAction
        enabled: main.hasItems && !main.isTransitioning
        visible: main.itemCount > 1
        text: i18nc("@action:button icon-only", "Show next image")
        icon.name: main.LayoutMirroring.enabled ? "go-next-rtl-symbolic" : "go-next-symbolic"
        onTriggered: {
            nextTimer.stop()
            main.nextItem()
        }
    }
    Kirigami.Action {
        id: openViewerAction
        icon.name: "document-preview"
        text: i18nc("@action:button icon-only", "Open in default image viewer")
        onTriggered: Qt.openUrlExternally(main.activeSource)
    }

    fullRepresentation: Item {
        id: fullRepresentation

        property Image frontImage: image1
        property Image bufferImage: image2

        Item {
            id: itemView
            anchors.fill: parent

            Item {
                id: imageView
                visible: main.hasItems
                anchors.fill: parent

                // This timer prevents reloading the image too often when resizing,
                // to minimize excessively re-reading the file on disk
                Timer {
                    id: imageReloadTimer
                    interval: 250
                    running: false
                    onTriggered: {
                        image1.sourceSize = Qt.size(image1.width, image1.height)
                        image2.sourceSize = Qt.size(image2.width, image2.height)
                    }
                }

                Image {
                    id: image2

                    anchors.fill: parent
                    z: image2 === fullRepresentation.frontImage
                    fillMode: Plasmoid.configuration.fillMode

                    opacity: 0

                    cache: false

                    asynchronous: true
                    autoTransform: true

                    onWidthChanged: imageReloadTimer.restart()
                    onHeightChanged: imageReloadTimer.restart()

                    sourceSize.width: width
                    sourceSize.height: height
                }

                Image {
                    id: image1

                    anchors.fill: parent
                    z: image2 === fullRepresentation.frontImage
                    fillMode: Plasmoid.configuration.fillMode

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
                        enabled: Plasmoid.configuration.leftClickOpenImage && !main.isTransitioning
                        onTapped: Qt.openUrlExternally(main.activeSource)
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
                OpacityAnimator { target: fullRepresentation.frontImage; from: 1; to: 0; duration: Kirigami.Units.veryLongDuration }
                OpacityAnimator { target: fullRepresentation.bufferImage; from: 0; to: 1; duration: Kirigami.Units.veryLongDuration }
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
        Binding { main.isTransitioning: faderAnimation.running }
        Connections {
            target: main
            function onRestartAnimationRequested() {
                faderAnimation.restart()
            }
        }

        DragDrop.DropArea {
            id: dropArea
            anchors.fill: parent

            onDrop: event => {
                var mimeData = event.mimeData
                if (mimeData.hasUrls) {
                    var urls = mimeData.urls
                    for (var i = 0, j = urls.length; i < j; ++i) {
                        var url = urls[i]
                        var type = items.isDir(url) ? "folder" : "file"
                        var item = { "path":url, "type":type }
                        main.addItem(item)
                    }
                }
                event.accept(Qt.CopyAction)
            }
        }

        Item {
            id: overlay

            anchors.fill: parent

            visible: main.hasItems
            opacity: overlayMouseArea.containsMouse ? 1 : 0

            Behavior on opacity {
                NumberAnimation {}
            }

            ButtonBar {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: Kirigami.Units.largeSpacing
                visible: main.itemCount > 1

                actions: [previousAction, openViewerAction, nextAction]
            }

            PlasmaComponents3.RoundButton {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: Kirigami.Units.smallSpacing
                visible: main.itemCount === 1
                display: PlasmaComponents3.RoundButton.IconOnly
                action: openViewerAction
                text: action.text
                icon.name: action.icon.name
                PlasmaComponents3.ToolTip.visible: hovered || visualFocus
                PlasmaComponents3.ToolTip.text: text
                PlasmaComponents3.ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            // BUG TODO Fix overlay so _all_ mouse events reach lower components
            MouseArea {
                id: overlayMouseArea

                anchors.fill: parent
                hoverEnabled: true

                propagateComposedEvents: true

                onPressed: mouse => mouse.accepted = false;
                onDoubleClicked: mouse => mouse.accepted = false;
                Binding { main.pause: overlayMouseArea.containsMouse }
            }

        }

        PlasmaExtras.PlaceholderMessage {
            id: configNeededPlaceholder
            property bool fitsInWidget: implicitWidth <= parent.width && implicitHeight <= parent.height
            anchors.centerIn: parent
            visible: !main.hasItems && fitsInWidget
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

            visible: !main.hasItems && !configNeededPlaceholder.fitsInWidget
            icon.name: "insert-image-symbolic"
            text: i18nc("@action:button opens settings dialog", "Choose media…")
            onClicked: {
                Plasmoid.internalAction("configure").trigger();
            }
        }
    }
}
