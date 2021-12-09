/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.newstuff 1.62 as NewStuff
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: root
    anchors.left: parent.left
    anchors.right: parent.right

    signal configurationChanged

    function saveConfig() {
        var newTabs = [];
        for (var i in providerColumn.children) {
            if (providerColumn.children[i].checked) {
                newTabs.push(providerColumn.children[i].plugin)
            }
        }
        plasmoid.nativeInterface.tabIdentifiers = newTabs;

        plasmoid.nativeInterface.middleClick = middleClickCheckBox.checked;

        plasmoid.nativeInterface.checkNewComicStripsInterval = checkNewComicStripsInterval.value;
        plasmoid.nativeInterface.providerUpdateInterval = providerUpdateInterval.value;

        plasmoid.nativeInterface.saveConfig();
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        middleClickCheckBox.checked = plasmoid.nativeInterface.middleClick;
        checkNewComicStripsInterval.value = plasmoid.nativeInterface.checkNewComicStripsInterval;
        providerUpdateInterval.value = plasmoid.nativeInterface.providerUpdateInterval
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    Layouts.ColumnLayout {
        id: providerColumn
        Kirigami.FormData.label: i18nc("@title:group", "Comics:")
        Kirigami.FormData.buddyFor: children[1] // 0 is the Repeater

        Repeater {
            model: plasmoid.nativeInterface.availableComicsModel
            delegate: Controls.CheckBox {
                id: checkbox
                text: model.display
                checked: model.checked
                property string plugin: model.plugin
                Component.onCompleted: {
                    checkbox.checked = plasmoid.nativeInterface.tabIdentifiers.indexOf(model.plugin) !== -1
                }
                onCheckedChanged: root.configurationChanged();
            }
        }
    }

    NewStuff.Button {
        id: ghnsButton
        text: i18nc("@action:button", "Get New Comics…")
        configFile: "comic.knsrc"
        onEntryEvent: function(entry, event) {
            if (event == 1) {
                plasmoid.nativeInterface.loadProviders()
            }
        }
    }

    Controls.CheckBox {
        id: middleClickCheckBox
        text: i18nc("@option:check", "Middle-click on comic to display at original size")
        onCheckedChanged: root.configurationChanged();
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    Layouts.RowLayout {
            Kirigami.FormData.label: i18nc("@label:spinbox", "Check for new plugins every:")

        Controls.SpinBox {
            id: providerUpdateInterval
            stepSize: 1
            onValueChanged: root.configurationChanged();
        }

        Controls.Label {
            text: i18ncp("@item:valuesuffix spacing to number + unit", "day", "days")
        }
    }

    Layouts.RowLayout {
            Kirigami.FormData.label: i18nc("@label:spinbox", "Check for new comics every:")

        Controls.SpinBox {
            id: checkNewComicStripsInterval
            stepSize: 1
            onValueChanged: root.configurationChanged();
        }

        Controls.Label {
            text: i18ncp("@item:valuesuffix spacing to number + unit (minutes)", "minute", "minutes")
        }
    }
}
