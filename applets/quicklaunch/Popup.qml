/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

import plasma.applet.org.kde.plasma.quicklaunch
import "layout.js" as LayoutManager

Item {
    id: popup

    property alias popupModel : popupModelObject
    property alias listView: listView
    required property Logic logic
    required property UrlModel launcherModel

    implicitWidth: LayoutManager.popupItemWidth()
    implicitHeight: Math.max(1, popupModel.count) * LayoutManager.popupItemHeight()

    QuickLaunchDropArea {
        id: dropArea
        anchors.fill: parent
        enabled: !Plasmoid.immutable

        model: listView.model
        view: listView

        onSaveConfigurationRequested: popup.saveConfiguration()
    }

    ListView {
        id: listView
        anchors.fill: parent

        focus: true
        interactive: true
        keyNavigationWraps: true

        model: UrlModel {
            id: popupModelObject
        }

        delegate: IconItem {
            isPopupItem: true
            popupModel: popupModelObject
            launcherModel: popup.launcherModel
            grid: null
            logic: popup.logic
            onMoveItemToGrid: (item, url) => listView.moveItemToGrid(item, url)
        }

        highlight: PlasmaExtras.Highlight {}

        highlightMoveDuration: 0

        function moveItemToGrid(iconItem, url) {
            popup.launcherModel.insertUrl(popup.launcherModel.count, url.toString());
            listView.currentIndex = popup.launcherModel.count - 1;
            iconItem.removeLauncher();
        }
    }

    Connections {
        target: Plasmoid.configuration
        function onPopupUrlsChanged() {
            if (dropArea.containsDrag) return;
            const configUrls = Plasmoid.configuration.popupUrls;
            const modelUrls = popupModelObject.urls();
            if (configUrls.length === modelUrls.length) {
                let same = true;
                for (let i = 0; i < configUrls.length; ++i) {
                    if (configUrls[i].toString() !== modelUrls[i].toString()) {
                        same = false;
                        break;
                    }
                }
                if (same) return;
            }
            popupModelObject.urlsChanged.disconnect(popup.saveConfiguration);
            popupModelObject.setUrls(Plasmoid.configuration.popupUrls);
            popupModelObject.urlsChanged.connect(popup.saveConfiguration);
        }
    }

    Component.onCompleted: {
        popupModelObject.setUrls(Plasmoid.configuration.popupUrls);
        popupModelObject.urlsChanged.connect(saveConfiguration);
    }

    function saveConfiguration()
    {
        if (!dropArea.containsDrag) {
            Plasmoid.configuration.popupUrls = popupModelObject.urls();
        }
    }
}
