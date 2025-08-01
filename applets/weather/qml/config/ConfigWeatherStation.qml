/*
 * SPDX-FileCopyrightText: 2016, 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: weatherStationConfigPage

    LocationsControl {
        id: locationListModel
        onLocationSearchDone: {
            isSearching = false;
        }
    }

    property string cfg_provider
    property string cfg_placeInfo
    property string cfg_placeDisplayName

    property alias cfg_updateInterval: updateIntervalSpin.value

    readonly property bool hasSource: cfg_provider !== "" && cfg_placeInfo !== ""
    readonly property bool canSearch: !!searchStringEdit.text && locationListModel.hasProviders

    // The model property `isValidatingInput` doesn't account for the timer delay
    // We use a custom property to provide a more responsive feedback
    property bool isSearching: false
    
    extraFooterTopPadding: true

    header: ColumnLayout {

        spacing: Kirigami.Units.smallSpacing

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
                opacity: hasSource ? 1 : 0.75

                text: {
                    if (hasSource) {
                        if (weatherStationConfigPage.cfg_placeDisplayName !== "") {
                            return weatherStationConfigPage.cfg_placeDisplayName;
                        }
                        return weatherStationConfigPage.cfg_placeInfo;
                    }
                    return i18nc("No location is currently selected", "None selected");
                }
                textFormat: Text.PlainText
            }

            QQC2.Label {
                Kirigami.FormData.label: hasSource ? i18nc("@label", "Provider:") : ""

                Layout.fillWidth: true
                elide: Text.ElideRight
                // Keep it visible to avoid height changes which can confuse AppletConfigurationPage
                opacity: hasSource ? 1 : 0

                text: {
                    if (weatherStationConfigPage.hasSource) {
                        return locationListModel.getProviderDisplayName(weatherStationConfigPage.cfg_provider);
                    }
                    return "";
                }
                textFormat: Text.PlainText
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
            enabled: locationListModel.hasProviders
            placeholderText: hasSource ? i18nc("@info:placeholder", "Enter new location") : i18nc("@info:placeholder", "Enter location")

            Timer {
                id: searchDelayTimer
                interval: 500
                onTriggered: {
                    if (!canSearch) {
                        locationListModel.clear();
                        return;
                    }
                    locationListModel.searchLocations(searchStringEdit.text);
                }
            }

            onTextChanged: {
                isSearching = text.length > 0
                searchDelayTimer.restart();
            }

            Keys.onUpPressed: event => {
                if (locationListView.currentIndex != 0) {
                    locationListView.currentIndex--;
                }
                event.accepted = true;
            }
            Keys.onDownPressed: event => {
                if (locationListView.currentIndex != locationListView.count - 1) {
                    locationListView.currentIndex++;
                }
                event.accepted = true;
            }
            Keys.onEscapePressed: event => {
                if (searchStringEdit.text.length === 0) {
                    event.accepted = false;
                    return;
                }
                searchStringEdit.clear();
                event.accepted = true;
            }
        }
    }

    view: ListView {
        id: locationListView
        model: locationListModel
        focus: true
        activeFocusOnTab: true
        keyNavigationEnabled: true
        enabled: locationListModel.hasProviders

        onCurrentIndexChanged: {
            const provider = locationListModel.getProviderByIndex(locationListView.currentIndex);
            const placeInfo = locationListModel.getPlaceInfoByIndex(locationListView.currentIndex);
            if (provider && placeInfo) {
                weatherStationConfigPage.cfg_provider = provider;
                weatherStationConfigPage.cfg_placeInfo = placeInfo;
                const placeDisplayName = locationListModel.getPlaceDisplayNameByIndex(locationListView.currentIndex);
                if (!!placeDisplayName && placeDisplayName !== "") {
                    weatherStationConfigPage.cfg_placeDisplayName = placeDisplayName
                } else {
                    weatherStationConfigPage.cfg_placeDisplayName = "";
                }
            }
        }

        delegate: QQC2.ItemDelegate {
            width: locationListView.width
            text: i18nc("A weather station location and the weather service it comes from",
                "%1 (%2)", !!model.displayName ? model.displayName : model.station, locationListModel.getProviderDisplayName(locationListModel.getProviderByIndex(model.index)))
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
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
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
            explanation: canSearch ? i18nc("@info:usagetip", "If you've used this weather station in the past, it's possible that a server outage at the weather station provider has made it temporarily unavailable. Try again later.") : ""

        }

        QQC2.BusyIndicator {
            id: busy
            anchors.centerIn: parent
            visible: locationListView.count === 0 && isSearching
        }
    }
}
