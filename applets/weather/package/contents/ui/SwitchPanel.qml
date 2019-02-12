/*
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

import QtQuick 2.9

import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2 as QtControls

import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {
    id: root

    property alias forecastModel: forecastView.model
    property alias detailsModel: detailsView.model
    property alias noticesModel: noticesView.model
    property alias forecastViewTitle: forecastTabButton.text

    readonly property bool hasDetailsContent: detailsModel && detailsModel.length > 0
    readonly property alias hasNoticesContent: noticesView.hasContent

    function removePage(page) {
        // fill-in for removeItem, replace for QQC >= 2.3
        for (var n = 0; n < swipeView.count; n++) {
            if (page === swipeView.itemAt(n)) {
                swipeView.removeItem(n);
                tabBar.itemAt(n).visible = false;
                tabBar.removeItem(n);
            }
        }
        page.visible = false
    }

    PlasmaComponents.TabButton {
        id: detailsTabButton

        visible: false;

        text: i18nc("@title:tab", "Details")
    }

    DetailsView {
        id: detailsView

        visible: false;

        model: detailsModel
    }

    PlasmaComponents.TabButton {
        id: noticesTabButton

        visible: false;

        text: i18nc("@title:tab", "Notices")
    }

    NoticesView {
        id: noticesView

        visible: false;

        model: noticesModel
    }

    PlasmaComponents.TabBar {
        id: tabBar

        Layout.fillWidth: true
        visible: hasDetailsContent || hasNoticesContent

        PlasmaComponents.TabButton {
            id: forecastTabButton
        }
    }


    QtControls.SwipeView {
        id: swipeView

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.minimumWidth: Math.max(forecastView.Layout.minimumWidth,
                                      detailsView.Layout.minimumWidth,
                                      noticesView.Layout.minimumWidth)
        Layout.minimumHeight: Math.max(forecastView.Layout.minimumHeight,
                                       detailsView.Layout.minimumHeight,
                                       noticesView.Layout.minimumHeight)

        clip: true // previous/next views are prepared outside of view, do not render them

        currentIndex: tabBar.currentIndex

        ColumnLayout {
            ForecastView {
                id: forecastView

                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                Layout.fillWidth: false
            }
        }

        onCurrentIndexChanged: {
            tabBar.setCurrentIndex(currentIndex);
        }
    }

    // perhaps 
    onHasDetailsContentChanged: {
        if (hasDetailsContent) {
            tabBar.insertItem(1, detailsTabButton);
            detailsTabButton.visible = true;
            swipeView.insertItem(1, detailsView);
            detailsView.visible = true
        } else {
            removePage(detailsView);
        }
    }

    onHasNoticesContentChanged: {
        if (hasNoticesContent) {
            tabBar.addItem(noticesTabButton);
            noticesTabButton.visible = true;
            swipeView.addItem(noticesView);
            noticesView.visible = true
        } else {
            removePage(noticesView);
        }
    }

    Component.onDestruction: {
        // prevent double deletion, take back inserted items
        if (hasDetailsContent) {
            removePage(detailsView);
        }
        if (hasNoticesContent) {
            removePage(noticesView);
        }
    }
}
