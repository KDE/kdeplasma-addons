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
import QtQuick.Controls 1.4 as QQC1
import org.kde.kquickcontrols 2.0 as KQuickControls
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: root
    twinFormLayouts: parentLayout
    anchors.left: parent.left
    anchors.right: parent.right

    property string cfg_Provider
    property int cfg_FillMode
    property alias cfg_Color: colorButton.color
    property alias formLayout: root

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
    // TODO: port to QQC2 version once we've fixed https://bugs.kde.org/show_bug.cgi?id=403153
    QQC1.ComboBox {
        id: providerComboBox
        TextMetrics {
            id: providerTextMetrics
            text: providerComboBox.currentText
        }
        implicitWidth: Math.max(providerTextMetrics.width + Kirigami.Units.gridUnit * 2 + Kirigami.Units.smallSpacing * 2, pluginComboBox.width) //QQC1 Combobox default sizing is broken
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Provider:")
        model: providerModel
        textRole: "name"
        onCurrentIndexChanged: {
            cfg_Provider = providerModel.get(currentIndex)["id"]
        }
    }

    // TODO: port to QQC2 version once we've fixed https://bugs.kde.org/show_bug.cgi?id=403153
    QQC1.ComboBox {
        id: resizeComboBox
        TextMetrics {
            id: resizeTextMetrics
            text: resizeComboBox.currentText
        }
        implicitWidth: Math.max(resizeTextMetrics.width + Kirigami.Units.gridUnit * 2 + Kirigami.Units.smallSpacing * 2, pluginComboBox.width) //QQC1 Combobox default sizing is broken
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Positioning:")
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

    KQuickControls.ColorButton {
        id: colorButton
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:chooser", "Background color:")
        dialogTitle: i18ndc("plasma_wallpaper_org.kde.potd", "@title:window", "Select Background Color")
    }
}
