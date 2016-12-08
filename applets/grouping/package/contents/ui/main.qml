/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *  Copyright 2016 David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.5
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0
import org.kde.draganddrop 2.0 as DnD
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles.Plasma 2.0 as Styles


import "items"

Item {
    id: root

    //be at least the same size as the system tray popup
    Layout.minimumWidth: units.gridUnit * 24
    Layout.minimumHeight: units.gridUnit * 21
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight * 1.5

    property Component plasmoidItemComponent

    Containment.onAppletAdded: addApplet(applet);
    Containment.onAppletRemoved: {
        for (var i=0; i<tabView.count; i++) {
            if (tabView.getTab(i).itemId == applet.id) {
                tabView.removeTab(i);
            }
        }
    }

    function addApplet(applet) {
        if (!plasmoidItemComponent) {
            plasmoidItemComponent = Qt.createComponent("items/PlasmoidItem.qml");
        }
        if (plasmoidItemComponent.status == Component.Error) {
            console.warn("Could not create PlasmoidItem", plasmoidItemComponent.errorString());
        }

        var plasmoidContainer = plasmoidItemComponent.createObject(tabView, {"applet": applet});

        applet.parent = plasmoidContainer;
        applet.anchors.fill = plasmoidContainer;
        applet.visible = true;
    }

    Component.onCompleted: {
        var applets = Containment.applets;
        for (var i =0 ; i < applets.length; i++) {
            addApplet(applets[i]);
        }
    }

    TabView {
        id: tabView
        style: Styles.TabViewStyle {}

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: false //we don't want to mirror applets that don't explicitly support it

        anchors.fill: parent
    }

    DnD.DropArea {
        anchors.fill: parent

        preventStealing: true;

        /** Extracts the name of the applet in the drag data if present
         * otherwise returns null*/
        function appletName(event) {
            if (event.mimeData.formats.indexOf("text/x-plasmoidservicename") < 0) {
                return null;
            }
            var plasmoidId = event.mimeData.getDataAsByteArray("text/x-plasmoidservicename");
            return plasmoidId;
        }

        onDragEnter: {
            if (!appletName(event)) {
                event.ignore();
            }
        }

        onDrop: {
            var plasmoidId = appletName(event);
            if (!plasmoidId) {
                event.ignore();
                return;
            }
            plasmoid.nativeInterface.newTask(plasmoidId);
        }
    }

    PlasmaComponents.Label {
        anchors.fill: tabView
        text: i18n("Drag applets here")
        visible: tabView.count == 0
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
