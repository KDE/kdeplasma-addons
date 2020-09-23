/*
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.8 as QQC2
import org.kde.kquickcontrols 2.0 as KQC2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: root
    twinFormLayouts: parentLayout

    property string cfg_Provider
    property string cfg_Category
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

    QQC2.ComboBox {
        id: providerComboBox
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Provider:")
        model: providerModel
        textRole: "name"
        onCurrentIndexChanged: {
            cfg_Provider = providerModel.get(currentIndex)["id"]
        }
    }

    QQC2.ComboBox {
        id: categoryComboBox
        visible: cfg_Provider === 'unsplash'
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Category:")
        model: [
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "All"),
                'value': '1065976'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "1080p"),
                'value': '1339107'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "4K"),
                'value': '1339090'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Ultra Wide"),
                'value': '1339089'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Background"),
                'value': '1339276'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Lock Screen"),
                'value': '1339070'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Nature"),
                'value': '1065376'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Tumblr"),
                'value': '1111644'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Black"),
                'value': '1101680'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Flower"),
                'value': '1100232'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Funny"),
                'value': '1111682'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Cute"),
                'value': '1111680'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Cool"),
                'value': '1111678'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Fall"),
                'value': '1100229'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Love"),
                'value': '1066280'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Design"),
                'value': '1066276'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Christmas"),
                'value': '1066273'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Travel"),
                'value': '1065428'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Beach"),
                'value': '1065423'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Car"),
                'value': '1065408'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Sports"),
                'value': '1065402'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Animal"),
                'value': '1065390'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "People"),
                'value': '1065386'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Music"),
                'value': '1065384'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Summer"),
                'value': '1065380'
            },
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Galaxy"),
                'value': '1065374'
            },
        ]
        textRole: "label"
        onCurrentIndexChanged: {
            cfg_Category = model[currentIndex]["value"]
        }
        Component.onCompleted: {
            for (var i = 0; i < model.length; i++) {
                if (model[i]["value"] == wallpaper.configuration.Category) {
                    categoryComboBox.currentIndex = i;
                }
            }
        }
    }

    QQC2.ComboBox {
        id: resizeComboBox
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

    KQC2.ColorButton {
        id: colorButton
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:chooser", "Background color:")
        dialogTitle: i18ndc("plasma_wallpaper_org.kde.potd", "@title:window", "Select Background Color")
    }
}
