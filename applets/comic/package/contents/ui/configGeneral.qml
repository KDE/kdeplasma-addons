/*
 *  Copyright 2015 Marco Martin <mart@kde.org>
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

import QtQuick 2.0
import QtQuick.Controls 1.1 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.plasma.core 2.0 as PlasmaCore


Layouts.ColumnLayout {
    id: root

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

    Controls.GroupBox {
        Layouts.Layout.fillWidth: true
        flat: true

        title: i18nc("@title:group", "Comic")

        Layouts.ColumnLayout {
            Layouts.ColumnLayout {
                id: providerColumn
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
            Item {
                Layouts.Layout.fillWidth: true
                Layouts.Layout.fillHeight: true
                Layouts.Layout.minimumHeight: units.gridUnit * 2
            }
            Controls.Button {
                iconName: "get-hot-new-stuff"
                text: i18nc("@action:button", "Get New Comics...")
                onClicked: plasmoid.nativeInterface.getNewComics();
            }
            Controls.CheckBox {
                id: middleClickCheckBox
                text: i18nc("@option:check", "Middle-click on the comic to show it at its original size")
                onCheckedChanged: root.configurationChanged();
            }
        }
    }
    Controls.GroupBox {
        Layouts.Layout.fillWidth: true
        flat: true

        title: i18nc("@title:group", "Update")

        Layouts.ColumnLayout {
            Layouts.RowLayout {
                Layouts.Layout.alignment: Qt.AlignRight
                Controls.Label {
                    text: i18nc("@label:spinbox", "Automatically update comic plugins:")
                }
                Controls.SpinBox {
                    id: providerUpdateInterval
                    Layouts.Layout.minimumWidth: units.gridUnit * 8
                    suffix: i18nc("@item:valuesuffix spacing to number + unit", " days")
                    stepSize: 1
                    onValueChanged: root.configurationChanged();
                }
            }
            Layouts.RowLayout {
                Layouts.Layout.alignment: Qt.AlignRight
                Controls.Label {
                    text: i18nc("@label:spinbox", "Check for new comic strips:")
                }
                Controls.SpinBox {
                    id: checkNewComicStripsInterval
                    Layouts.Layout.minimumWidth: units.gridUnit * 8
                    suffix: i18nc("@item:valuesuffix spacing to number + unit (minutes)", " min")
                    stepSize: 1
                    onValueChanged: root.configurationChanged();
                }
            }
        }
    }

    Item {
        Layouts.Layout.fillHeight: true
    }
}
