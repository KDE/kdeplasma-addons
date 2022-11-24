/*
 * SPDX-FileCopyrightText: 2016, 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.weather 1.0


ColumnLayout {
    id: weatherStationConfigPage

    property string cfg_source
    property alias cfg_updateInterval: updateIntervalSpin.value

    readonly property var providers: Plasmoid.nativeInterface.providers

    readonly property var sourceDetails: cfg_source ? cfg_source.split('|') : ""
    readonly property bool hasSource: sourceDetails.length > 2
    readonly property bool canSearch: !!searchStringEdit.text && Object.keys(providers).length

    // The model property `isValidatingInput` doesn't account for the timer delay
    // We use a custom property to provide a more responsive feedback
    property bool isSearching: false

    LocationListModel {
        id: locationListModel
        onLocationSearchDone: {
            isSearching = false;
        }
    }

    Kirigami.FormLayout {
        id: formLayout

        QQC2.SpinBox {
            id: updateIntervalSpin

            Kirigami.FormData.label: i18nc("@label:spinbox", "Update every:")

            textFromValue: function(value) {
                return (i18np("%1 minute", "%1 minutes", value));
            }
            valueFromText: function(text) {
                return parseInt(text);
            }

            from: 30
            to: 3600
            editable: true
        }

        QQC2.Label {
            Kirigami.FormData.label: i18nc("@label", "Location:")

            Layout.fillWidth: true
            elide: Text.ElideRight
            opacity: hasSource ? 1 : 0.7

            text: hasSource ? sourceDetails[2] : i18nc("No location is currently selected", "None selected")
        }

        QQC2.Label {
            Kirigami.FormData.label: hasSource ? i18nc("@label", "Provider:") : ""

            Layout.fillWidth: true
            elide: Text.ElideRight
            // Keep it visible to avoid height changes which can confuse AppletConfigurationPage
            opacity: hasSource ? 1 : 0

            text: hasSource ? providers[sourceDetails[0]] : ""
        }

        Item {
            // This dummy item adds some spacing and makes the form layout less jumpy
            Kirigami.FormData.isSection: true
            implicitWidth: Math.min(Kirigami.Units.gridUnit * 15, weatherStationConfigPage.width - Kirigami.Units.gridUnit)
        }
    }

    Kirigami.SearchField {
        id: searchStringEdit

        Layout.fillWidth: true

        focus: true
        enabled: Object.keys(providers).length > 0
        placeholderText: hasSource ? i18nc("@info:placeholder", "Enter new location") : i18nc("@info:placeholder", "Enter location")

        Timer {
            id: searchDelayTimer
            interval: 500
            onTriggered: {
                if (!canSearch) {
                    locationListModel.clear();
                    return;
                }
                locationListModel.searchLocations(searchStringEdit.text, Object.keys(providers));
            }
        }

        onTextChanged: {
            isSearching = text.length > 0
            searchDelayTimer.restart();
        }

        Keys.onUpPressed: {
            if (locationListView.currentIndex != 0) {
                locationListView.currentIndex--;
            }
            event.accepted = true;
        }
        Keys.onDownPressed: {
            if (locationListView.currentIndex != locationListView.count - 1) {
                locationListView.currentIndex++;
            }
            event.accepted = true;
        }
        Keys.onEscapePressed: {
            if (searchStringEdit.text.length === 0) {
                event.accepted = false;
                return;
            }
            searchStringEdit.clear();
            event.accepted = true;
        }
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.maximumHeight: weatherStationConfigPage.height - formLayout.height - searchStringEdit.height - 3 * Kirigami.Units.smallSpacing

        Component.onCompleted: {
            background.visible = true;
        }

        enabled: Object.keys(providers).length > 0

        ListView {
            id: locationListView
            model: locationListModel
            focus: true
            activeFocusOnTab: true
            keyNavigationEnabled: true

            onCurrentIndexChanged: {
                const source = locationListModel.valueForListIndex(locationListView.currentIndex);
                if (source) {
                     weatherStationConfigPage.cfg_source = source;
                }
            }

            delegate: QQC2.ItemDelegate {
                width: locationListView.width
                text: model.display
                highlighted: ListView.isCurrentItem

                onClicked: {
                    locationListView.forceActiveFocus();
                    locationListView.currentIndex = index;
                }
            }

            // To avoid start with a highlighted item on the next search
            onCountChanged: {
                if (count === 0) {
                    currentIndex = -1;
                }
            }

            Keys.forwardTo: searchStringEdit

            Kirigami.PlaceholderMessage {
                id: listViewPlaceholder
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit
                visible: locationListView.count === 0 && !isSearching
                text: {
                    if (canSearch) {    // There is a search text
                        return i18nc("@info", "No weather stations found for '%1'", searchStringEdit.text);
                    } else if (hasSource) {
                        return i18nc("@info", "Search for a weather station to change your location");
                    } else {
                        return i18nc("@info", "Search for a weather station to set your location");
                    }
                }

            }

            QQC2.BusyIndicator {
                id: busy
                anchors.centerIn: parent
                visible: locationListView.count === 0 && isSearching
            }
        }
    }
}
