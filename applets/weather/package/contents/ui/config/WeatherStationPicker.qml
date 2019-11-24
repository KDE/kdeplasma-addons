/*
 * Copyright 2016,2018  Friedrich W. H. Kossebau <kossebau@kde.org>
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

import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.plasma.private.weather 1.0


ColumnLayout {
    id: root

    property alias selectedServices: serviceListModel.selectedServices
    property string source
    readonly property bool canSearch: !!searchStringEdit.text && selectedServices.length
    readonly property bool handlesEnterKey: canSearch && searchStringEdit.activeFocus

    function searchLocation() {
        if (!canSearch) {
            return;
        }

        // avoid automatic selection once model is refilled
        locationListView.currentRow = -1;
        locationListView.selection.clear();
        noSearchResultReport.visible = false;
        source = "";
        locationListView.forceActiveFocus();

        locationListModel.searchLocations(searchStringEdit.text, selectedServices);
    }

    function handleLocationSearchDone(success, searchString) {
        if (!success) {
            noSearchResultReport.text = i18nc("@info", "No weather stations found for '%1'", searchString);
            noSearchResultReport.visible = true;
        }
    }

    LocationListModel {
        id: locationListModel
        onLocationSearchDone: handleLocationSearchDone(success, searchString);
    }

    ServiceListModel {
        id: serviceListModel
    }

    QQC2.Menu {
        id: serviceSelectionMenu

        Instantiator {
            model: serviceListModel
            delegate: QQC2.MenuItem {
                text: model.display
                checkable: true
                checked: model.checked

                onToggled: {
                    model.checked = checked;
                    checked = Qt.binding(function() { return model.checked; });
                    weatherStationConfigPage.configurationChanged();
                }
            }
            onObjectAdded: serviceSelectionMenu.insertItem(index, object)
            onObjectRemoved: serviceSelectionMenu.removeItem(object)
        }

    }

    RowLayout {
        Layout.fillWidth: true

        QQC2.TextField {
            id: searchStringEdit

            Layout.fillWidth: true
            Layout.minimumWidth: implicitWidth
            placeholderText: i18nc("@info:placeholder", "Enter location")
            onAccepted: {
                searchLocation();
            }
        }

        QQC2.Button {
            id: serviceSelectionButton

            icon.name: "services"

            checkable: true
            checked: serviceSelectionMenu.opened
            onClicked: serviceSelectionMenu.popup(serviceSelectionButton, serviceSelectionButton.width - serviceSelectionMenu.width, serviceSelectionButton.height)

            QQC2.ToolTip {
                text: i18nc("@info:tooltip", "Select weather services providers")
                visible: hovered
            }
        }

        QQC2.Button {
            id: searchButton

            icon.name: "edit-find"
            text: i18nc("@action:button", "Search")
            enabled: canSearch

            onClicked: {
                searchLocation();
            }
        }
    }

    QQC1.TableView {
        id: locationListView

        Layout.minimumWidth: implicitWidth
        Layout.minimumHeight: implicitHeight
        Layout.fillWidth: true
        Layout.fillHeight: true

        headerVisible: false
        model: locationListModel

        onActivated: {
            if (row !== -1 && rowCount) {
                source = locationListModel.valueForListIndex(row);
            }
        }

        QQC1.TableViewColumn {
            id: locationListViewStationColumn

            movable: false
            resizable: false
            role: "display"
        }

        QQC2.Label {
            id: noSearchResultReport

            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            visible: false
        }

        QQC2.BusyIndicator {
            id: busy

            anchors.centerIn: parent
            width: Kirigami.Units.gridUnit
            height: width

            visible: locationListModel.validatingInput
        }
    }

    Component.onCompleted: {
        searchStringEdit.forceActiveFocus();
    }
}
