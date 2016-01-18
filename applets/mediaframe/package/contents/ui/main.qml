/*
 *  Copyright 2015  Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0

import QtMultimedia 5.0

import org.kde.draganddrop 2.0 as DragDrop

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.private.mediaframe 2.0

import "../code/utility.js" as Utility

Item {
    id: main

    MediaFrame {
        id: items
        random: true
    }
    
    property int implicitWidth: units.gridUnit * 30
    property int implicitHeight: units.gridUnit * 20
    
    Plasmoid.preferredRepresentation: plasmoid.fullRepresentation
    
    Plasmoid.switchWidth: units.gridUnit * 5
    Plasmoid.switchHeight: units.gridUnit * 5
    
    Plasmoid.backgroundHints: config.useBackground ? 1 : "NoBackground"
    
    property int minimumWidth: theme.mSize(theme.defaultFont).width * 45
    property int minimumHeight: theme.mSize(theme.defaultFont).height * 18
    
    width: minimumWidth
    height: minimumHeight

    property var config: {}
    
    property string activeSource: ""
    property string transitionSource: ""
    
    property var history: []
    property var future: []
    
    property bool pause: false
    
    property int itemCount: (items.count + future.length)
    property bool hasItems: ((itemCount > 0) || (future.length > 0))
    property bool isTransitioning: faderAnimation.running
    
    Component.onCompleted: {
        loadConfig()
        loadPathList()
    }
    
    onActiveSourceChanged: {
        console.debug('active source',activeSource)
        items.watch(activeSource)
    }
    
    onPauseChanged: {
        //console.debug('paused',pause)
    }
    
    onHasItemsChanged: {
        if(hasItems) {
            if(activeSource == '')
                nextItem()
        }
    }
    
    function addItem(item) {
        
        if(items.has(item.path)) {
            console.info(item.path,'already exists. Skipping...')
            return
        }
        
        var t = []
        if(plasmoid.configuration.pathList != '')
            t = JSON.parse(plasmoid.configuration.pathList)
        t.push(item)
        plasmoid.configuration.pathList = JSON.stringify(t)
    }
    
    function nextItem() {
        
        if(!hasItems) {
            console.warn('No items available')
            return
        }
        
        var active = activeSource
        
        // Only record history if we have more than one item
        if(itemCount > 1)
            pushHistory(active)
        
        if(future.length > 0) {
            setActiveSource(popFuture())
            countDownTimer.restart()
        } else {
            //setLoading()
            items.get(function(filePath){
                setActiveSource(filePath)
                countDownTimer.restart()
                //unsetLoading()
            },function(errorMessage){
                //unsetLoading()
                console.error('Error while getting next image',errorMessage)
            })
        }
        
        
    }
    
    function previousItem() {
        var active = activeSource
        pushFuture(active)
        var filePath = popHistory()
        setActiveSource(filePath)
    }
    
    function blacklistItem() {
        // TODO
    }
    
    function loadConfig() {
        config = {
            //pathList: 
            interval: plasmoid.configuration.interval,
            randomize: plasmoid.configuration.randomize,
            pauseOnMouseOver: plasmoid.configuration.pauseOnMouseOver,
            useBackground: plasmoid.configuration.useBackground,
            leftClickOpenImage: plasmoid.configuration.leftClickOpenImage,
            showCountdown: plasmoid.configuration.showCountdown
        }
    }
    
    function loadPathList() {
        var json = plasmoid.configuration.pathList
        if(json != '') {
            var list = JSON.parse(json)
            items.clear()
            for(var i in list) {
                var item = list[i]
                items.add(item.path,true)
            }
        }
        
    }
    
    function pushHistory(entry) {
        if(entry != "") {
            console.debug('pushing to history',entry)
            // TODO (move to native code?)
            // Rather nasty trick to let QML know that the array has changed
            // We do this because we're doing actions based on the .length property
            var t = history 
            t.push(entry)
            history = t
        }
    }
    
    function popHistory() {
        // NOTE see comment in "pushHistory"
        var t = history
        var entry = t.pop()
        history = t
        console.debug('poping from history',entry)
        return entry
    }
    
    function pushFuture(entry) {
        if(entry != "") {
            console.debug('pushing to future',entry)
            // NOTE see comment in "pushHistory"
            var t = future 
            t.push(entry)
            future = t
        }
    }
    
    function popFuture() {
        // NOTE see comment in "pushHistory"
        var t = future
        var entry = t.pop()
        future = t
        console.debug('poping from future',entry)
        return entry
    }
    
    Connections {
        target: items

        onItemChanged: {
            console.log('item',path,'changed')
            activeSource = ""
            setActiveSource(path)
        }
        
    }
    
    Connections {
        target: plasmoid.configuration

        onPathListChanged: {
            loadPathList()
        }
        
        onIntervalChanged: {
            loadConfig()
        }
        
        onRandomizeChanged: {
            loadConfig()
        }
        
        onPauseOnMouseOverChanged: {
            loadConfig()
        }
        
        onUseBackgroundChanged: {
            loadConfig()
        }
        
        onLeftClickOpenImageChanged: {
            loadConfig()
        }
        
        onShowCountdownChanged: {
            loadConfig()
        }
    }
    
    Timer {
        id: nextTimer
        interval: config.interval
        repeat: true
        running: hasItems && !pause
        triggeredOnStart: false
        onTriggered: { nextItem() }
    }
    
    Item {
        id: itemView
        anchors.fill: parent
        
        /*
        Video {
            id: video
            width : 800
            height : 600
            source: ""

            onStatusChanged: {
                if(status == Video.Loaded)
                    video.play()
            }
        }
        */
        
        Item {
            id: imageView
            visible: hasItems
            anchors.fill: parent
            
            Image {
                id: bufferImage
                
                
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                opacity: 0
                
                cache: false
                source: transitionSource
                
            }
            
            Image {
                id: frontImage
                
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                
                cache: false
                source: activeSource
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: { if(config.leftClickOpenImage) Qt.openUrlExternally(activeSource) }
                }
                
            }
        }
        
        // BUG TODO fix the rendering of the drop shadow
        DropShadow {
            id: itemViewDropShadow
            anchors.fill: parent
            visible: imageView.visible && !config.useBackground
            
            radius: 8.0
            samples: 16
            color: "#80000000"
            source: frontImage
        }
        
    }
    
    function setActiveSource(source) {
        if(itemCount > 1) { // Only do transition if we have more that one item
            transitionSource = source
            faderAnimation.restart()
        } else {
            transitionSource = source
            activeSource = source
        }
    }
    
    SequentialAnimation {
        id: faderAnimation
        
        ParallelAnimation {
            NumberAnimation { target: frontImage; property: "opacity"; to: 0; duration: 450 }
            NumberAnimation { target: bufferImage; property: "opacity"; to: 1; duration: 450 }
        }
        ScriptAction { script: {
            // Copy the transitionSource
            var ts = transitionSource
            activeSource = ts
            frontImage.opacity = 1
            transitionSource = ""
            bufferImage.opacity = 0
        }}
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
                    var type = 'file'
                    if(items.isDir(url))
                        type = 'folder'
                    var item = { 'path':url, 'type':type }
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
        opacity: 0
        
        Behavior on opacity {
            NumberAnimation {}
        }
        
        PlasmaComponents.Button {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            enabled: (history.length > 0) && !isTransitioning
            iconSource: 'arrow-left'
            onClicked: { nextTimer.stop(); previousItem() }
        }
        
        PlasmaComponents.Button {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            enabled: hasItems && !isTransitioning
            iconSource: 'arrow-right'
            onClicked: { nextTimer.stop(); nextItem() }
        }
        
        Row {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 6
            
            /*
            PlasmaComponents.Button {
                iconSource: 'documentinfo'
                onClicked: {  }
            }
            */
            PlasmaComponents.Button {
                
                //text: activeSource.split("/").pop().slice(-25)
                iconSource: 'document-preview'
                onClicked: { Qt.openUrlExternally(main.activeSource) }
                //tooltip: activeSource
            }
            /*
            PlasmaComponents.Button {
                iconSource: 'trash-empty'
                onClicked: {  }
            }
            
            PlasmaComponents.Button {
                iconSource: 'flag-black'
                onClicked: {  }
            }
            */
        }
        
        // BUG TODO Fix overlay so _all_ mouse events reach lower components
        MouseArea {
            id: overlayMouseArea
            
            anchors.fill: parent
            hoverEnabled: true
            
            propagateComposedEvents: true
            
            onEntered: {
                overlay.opacity = 1
                if(config.pauseOnMouseOver) main.pause = true
            }
            
            onExited: {
                overlay.opacity = 0
                if(config.pauseOnMouseOver) main.pause = false
            }
            
            //onClicked: mouse.accepted = false;
            onPressed: mouse.accepted = false;
            //onReleased: mouse.accepted = false;
            onDoubleClicked: mouse.accepted = false;
            //onPositionChanged: mouse.accepted = false;
            //onPressAndHold: mouse.accepted = false;
            
        }
        
    }
    
    // Visualization of the count down
    
    // TODO Find a nicer count down visualization
    // - maybe based on a QML Animation to offload to an animation thread
    // - don't use a timer
    Timer {
        id: countDownTimer
        
        property real elapsed: 0
        property real steps: config.interval / 500
        
        interval: config.interval / steps
        repeat: true
        running: nextTimer.running
        triggeredOnStart: true
        onTriggered: {
            progressBar.value = Utility.remap(elapsed,0,steps,0,1)
            elapsed++
        }
        onRunningChanged: {
            if(!running)
                elapsed = 0
        }
    }
    
    ProgressBar {
        id: progressBar
        
        visible: config.showCountdown && hasItems && itemCount > 1
        
        value: 0
        style: ProgressBarStyle {
            panel : Rectangle
            {
                color: "transparent"
                implicitWidth: Math.max(main.width, main.height) / 20
                implicitHeight: implicitWidth

                Rectangle
                {
                    id: outerRing
                    z: 0
                    anchors.fill: parent
                    
                    opacity:  pause ? 0.1 : 0.5
                    
                    radius: Math.max(width, height) / 2
                    color: "transparent"
                    border.color: "gray"
                    border.width: 8
                }

                Rectangle
                {
                    id: innerRing
                    z: 1
                    anchors.fill: parent
                    anchors.margins: (outerRing.border.width - border.width) / 2
                    
                    opacity:  pause ? 0.1 : 0.5
                    
                    radius: outerRing.radius
                    color: "transparent"
                    border.color: "lightblue"
                    border.width: 4

                    ConicalGradient
                    {
                        source: innerRing
                        anchors.fill: parent
                        gradient: Gradient
                        {
                            GradientStop { position: 0.00; color: "white" }
                            GradientStop { position: control.value; color: "white" }
                            GradientStop { position: control.value + 0.01; color: "transparent" }
                            GradientStop { position: 1.00; color: "transparent" }
                        }
                    }
                }
                
                PlasmaCore.IconItem {
                    id: pauseIcon
                    visible: pause
                    anchors.fill: parent
                    source: "media-playback-pause"
                    colorGroup: PlasmaCore.ColorScope.colorGroup

                    /*
                    PlasmaComponents.BusyIndicator {
                        id: connectingIndicator

                        anchors.fill: parent
                        running: hasItems
                        visible: running
                    }
                    */
                }
                /*
                Text
                {
                    id: progressLabel
                    anchors.centerIn: parent
                    color: "black"
                    text: (control.value * 100).toFixed() + "%"
                }
                */
            }
        }
    }
        
    PlasmaComponents.Button {
        
        anchors.centerIn: parent
        
        visible: !hasItems
        iconSource: "configure"
        text: "Configure plasmoid"
        onClicked: {
            plasmoid.action("configure").trigger();
        }
    }
    
}
