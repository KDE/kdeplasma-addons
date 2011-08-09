/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    width: 200
    height: 300
    id: devicenotifier
    property bool removableDevices: true
    property bool nonRemovableDevices: false
    property bool allDevices: false

    PlasmaCore.DataSource {
        id: hpSource
        engine: "hotplug"
        connectedSources: []
        interval: 0
        onSourceAdded: addDevice(source)
        onSourceRemoved: disconnectSource (source)
    }

    PlasmaCore.DataSource {
        id: sdSource
        engine: "soliddevice"
        connectedSources: hpSource.sources
        interval: 0
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged);
        plasmoid.popupIcon = QIcon("device-notifier");
        //configChanged();
    }

    function configChanged() {
        removableDevices = plasmoid.readConfig("removableDevices");
        nonRemovableDevices = plasmoid.readConfig("nonRemovableDevices");
        allDevices = plasmoid.readConfig("allDevices");
        populateDevices();
        notifierDialog.currentIndex = -1;
        notifierDialog.currentExpanded = -1;
    }

    function addDevice(source) {
        if (hpSource.connectedSources.indexOf(source)>=0)
            return;
        print ("===="+source);
        sdSource.connectSource (source);
        if (removableDevices) { //Removable only
            if (!sdSource.data[source]["Removable"]) {
                return;
            }
        }
        else if (nonRemovableDevices) { //Non removable only
            if (sdSource.data[source]["Removable"]) {
                return;
            }
        }
        hpSource.connectSource(source);
        i = notifierDialog.model.indexOf (source);
        notifierDialog.currentExpanded = i;
        notifierDialog.currentIndex = i;
        notifierDialog.highlightItem.opacity = 1;
    }

    function populateDevices() {
        // remove devices that might not belong to the newly
        // configured device type to show
        var connected = hpSource.connectedSources;
        for (i=0; i<connected.length; i++)
            hpSource.disconnectSource(connected[i]);

        // add each device again
        var sources = hpSource.sources;
        for (i=0; i<sources.length; i++) {
            addDevice(sources[i]);
        }
    }

    Text {
        id: header
        text: notifierDialog.model.length>0 ? "Available Devices" : "No Devices Available"
        anchors { top: parent.top; left: parent.left; right: parent.right }
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaWidgets.Separator {
        id: separator
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        anchors { topMargin: 3 }
    }

    ListView {
        id: notifierDialog
        anchors {
            top : separator.bottom
            topMargin: 10
            bottom: devicenotifier.bottom
        }
        model: hpSource.connectedSources
        delegate: deviceItem
        highlight: deviceHighlighter

        property int currentExpanded: -1
        Component.onCompleted: currentIndex=-1
    }

    Component {
        id: deviceItem

        DeviceItem {
            id: wrapper
            width: devicenotifier.width
            udi: modelData
            icon: hpSource.data[udi]["icon"]
            deviceName: hpSource.data[udi]["text"]
            emblemIcon: sdSource.data[udi]["Emblems"][0]
            
            percentUsage: {
                var freeSpace = Number(sdSource.data[udi]["Free Space"]);
                var size = Number(sdSource.data[udi]["Size"]);
                var used = size-freeSpace;
                return used*100/size;
            }
            leftActionIcon: {
                if (emblemIcon == "emblem-mounted") {
                    return QIcon("media-eject");
                } else if (emblemIcon == "emblem-unmounted") {
                    return QIcon("emblem-mounted");
                }
                else return QIcon("");
            }
            mounted: emblemIcon=="emblem-mounted"

            onLeftActionTriggered: {
                operationName = mounted ? "unmount" : "mount";
                service = sdSource.serviceForSource (udi);
                operation = service.operationDescription (operationName);
                service.startOperationCall (operation);
            }
        }
    }

    Component {
        id: deviceHighlighter

        PlasmaCore.FrameSvgItem {
            width: devicenotifier.width
            imagePath: "widgets/viewitem"
            prefix: "hover"
            opacity: 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
    }

    function isMounted (udi) {
        var types = sdSource.data[udi]["Device Types"];
        if (types.indexOf("Storage Access")>=0) {
            if (sdSource.data[udi]["Accessible"]) {
                return true;
            }
            else {
                return false;
            }
        }
        else if (types.indexOf("Storage Volume")>=0 && types.indexOf("OpticalDisc")>=0) {
            return true;
        }
        else {
            return false;
        }
    }
}
