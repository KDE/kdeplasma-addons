/*
 *   Copyright 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0 as QtControls1
import QtQuick.Controls 2.0 as QtControls
import org.kde.kquickcontrols 2.0 as KQuickControls
import org.kde.plasma.core 2.0 as PlasmaCore

ColumnLayout {
    id: root
    property string cfg_Provider
    property int cfg_FillMode
    property alias cfg_Color: colorButton.color
    ListModel {
        id: providerModel
    }

    function populateProviders() {
        providerModel.clear();
        var providers = engine.data["Providers"];
        if (providers) {
            var provider;
            for (provider in providers) {
                providerModel.append({'id': provider, 'name': providers[provider]})
            }
        }
    }

    PlasmaCore.DataSource {
        id: engine
        engine: "potd"
        connectedSources: ["Providers"]
        onDataChanged: populateProviders()
    }

    Component.onCompleted: {
        populateProviders()
        for (var i = 0; i < providerModel.count; i++) {
            if (providerModel.get(i)["id"] == wallpaper.configuration.Provider) {
                providerComboBox.currentIndex = i;
                break;
            }
        }
    }

    Row {
        spacing: units.largeSpacing / 2
        QtControls.Label {
            width: formAlignment - units.largeSpacing
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: providerComboBox.verticalCenter
            text: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Providers:")
        }
        // TODO: port to QQC2 version once we've fixed https://bugs.kde.org/show_bug.cgi?id=403153
        QtControls1.ComboBox {
            id: providerComboBox
            property int textLength: 24
            width: theme.mSize(theme.defaultFont).width * textLength
            model: providerModel
            textRole: "name"
            onCurrentIndexChanged: {
                cfg_Provider = providerModel.get(currentIndex)["id"]
            }
        }
    }

    Row {
        //x: formAlignment - positionLabel.paintedWidth
        spacing: units.largeSpacing / 2
        QtControls.Label {
            width: formAlignment - units.largeSpacing
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: resizeComboBox.verticalCenter
            text: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Positioning:")
        }
        // TODO: port to QQC2 version once we've fixed https://bugs.kde.org/show_bug.cgi?id=403153
        QtControls1.ComboBox {
            id: resizeComboBox
            property int textLength: 24
            width: theme.mSize(theme.defaultFont).width * textLength
            model: [
                        {
                            'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Scaled and Cropped"),
                            'fillMode': Image.PreserveAspectCrop
                        },
                        {
                            'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Scaled"),
                            'fillMode': Image.Stretch
                        },
                        {
                            'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Scaled, Keep Proportions"),
                            'fillMode': Image.PreserveAspectFit
                        },
                        {
                            'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Centered"),
                            'fillMode': Image.Pad
                        },
                        {
                            'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Tiled"),
                            'fillMode': Image.Tile
                        }
                    ]

            textRole: "label"
            onCurrentIndexChanged: cfg_FillMode = model[currentIndex]["fillMode"]
            Component.onCompleted: setMethod();

            function setMethod() {
                for (var i = 0; i < model.length; i++) {
                    if (model[i]["fillMode"] == wallpaper.configuration.FillMode) {
                        resizeComboBox.currentIndex = i;
                        var tl = model[i]["label"].length;
                    }
                }
            }
        }
    }

    Row {
        id: colorRow
        spacing: units.largeSpacing / 2
        QtControls.Label {
            width: formAlignment - units.largeSpacing
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: colorButton.verticalCenter
            text: i18ndc("plasma_wallpaper_org.kde.potd", "@label:chooser", "Background color:")
        }
        KQuickControls.ColorButton {
            id: colorButton
            dialogTitle: i18ndc("plasma_wallpaper_org.kde.potd", "@title:window", "Select Background Color")
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
