/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.1

import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: fullRoot

    Layout.minimumWidth: units.gridUnit * 12
    Layout.minimumHeight: units.gridUnit * 12
    Layout.preferredWidth: Layout.minimumWidth * 1.5
    Layout.preferredHeight: Layout.minimumHeight * 1.5

    TopPanel {
        id: panel

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            // matching round ends of bars behind data rows
            margins: units.smallSpacing
        }
        height: parent.height * 0.21
        model: plasmoid.nativeInterface.panelModel
    }

    PlasmaComponents.TabBar {
        id: tabBar

        anchors {
            top: panel.bottom
            topMargin: units.smallSpacing
            horizontalCenter: parent.horizontalCenter
        }

        visible: detailsView.hasContent || noticesView.hasContent

        function fallBackToFiveDaysIfInvisble(tabButton) {
            // PlasmaComponents.TabBar does not handle this (yet), so let's do it ourselves
            if (!tabButton.visible && (currentTab == tabButton)) {
                currentTab = fiveDaysTabButton;
                mainTabGroup.currentTab = fiveDaysTabButton.tab;
            }
        }

        PlasmaComponents.TabButton {
            id: fiveDaysTabButton

            text: plasmoid.nativeInterface.panelModel.totalDays
            tab: fiveDaysView
        }
        PlasmaComponents.TabButton {
            id: detailsTabButton

            text: i18n("Details")
            visible: detailsView.hasContent
            tab: detailsView
            onVisibleChanged: {
                tabBar.fallBackToFiveDaysIfInvisble(detailsTabButton);
            }
        }
        PlasmaComponents.TabButton {
            id: noticesTabButton

            text: i18n("Notices")
            visible: noticesView.hasContent
            tab: noticesView
            onVisibleChanged: {
                tabBar.fallBackToFiveDaysIfInvisble(noticesTabButton);
            }
        }
    }

    PlasmaComponents.TabGroup {
        id: mainTabGroup

        anchors {
            top: tabBar.visible ? tabBar.bottom : tabBar.top
            bottom: courtesyLabel.top
            left: parent.left
            right: parent.right
            topMargin: units.smallSpacing
            bottomMargin: units.smallSpacing
        }

        FiveDaysView {
            id: fiveDaysView
            anchors.fill: parent
            model: plasmoid.nativeInterface.fiveDaysModel
        }

        DetailsView {
            id: detailsView
            anchors.fill: parent
            model: plasmoid.nativeInterface.detailsModel
        }

        NoticesView {
            id: noticesView
            anchors.fill: parent
            model: plasmoid.nativeInterface.noticesModel
        }
    }

    PlasmaComponents.Label {
        id: courtesyLabel

        property string creditUrl: plasmoid.nativeInterface.panelModel.creditUrl

        anchors {
            bottom: parent.bottom
            right: parent.right
            left: parent.left
            // matching round ends of bars behind data rows
            rightMargin: units.smallSpacing
        }
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignRight
        font {
            pointSize: theme.smallestFont.pointSize
            underline: !!creditUrl
        }
        linkColor : color
        opacity: 0.6
        textFormat: Text.StyledText
        text: {
            var result = plasmoid.nativeInterface.panelModel.courtesy;
            if (creditUrl) {
                result = "<a href=\"" + creditUrl + "\">" + result + "</a>";
            }
            return result;
        }

        onLinkActivated: {
            Qt.openUrlExternally(link);
        }
    }
}
