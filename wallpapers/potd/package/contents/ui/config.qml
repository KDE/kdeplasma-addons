/*
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.8 as QQC2
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.kde.kcm 1.6 as KCM
import org.kde.kquickcontrols 2.0 as KQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.wallpapers.potd 1.0

Kirigami.FormLayout {
    id: root
    twinFormLayouts: parentLayout

    property string cfg_Provider
    property string cfg_Category
    property int cfg_FillMode
    property alias cfg_Color: colorButton.color
    property int cfg_UpdateOverMeteredConnection
    property alias formLayout: root

    PotdBackend {
        id: backend
        identifier: cfg_Provider
        arguments: {
            if (identifier === "unsplash") {
                // Needs to specify category for unsplash provider
                return [cfg_Category];
            } else if (identifier === "bing") {
                // Bing supports 1366/1920/UHD resolutions
                const w = Screen.width * Screen.devicePixelRatio > 1920 ? 3840 : 1920;
                const h = Screen.height * Screen.devicePixelRatio > 1080 ? 2160 : 1080;
                return [w, h];
            }
            return [];
        }
        updateOverMeteredConnection: wallpaper.configuration.UpdateOverMeteredConnection
    }

    Row {
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Provider:")

        QQC2.ComboBox {
            id: providerComboBox
            model: PotdProviderModel { }
            currentIndex: model.indexOf(cfg_Provider)
            textRole: "display"
            valueRole: "id"
            onCurrentValueChanged: {
                if (currentIndex < 0) {
                    return;
                }
                cfg_Provider = currentValue;
            }
        }

        KCM.ContextualHelpButton {
            anchors.verticalCenter: providerComboBox.verticalCenter
            visible: providerComboBox.model.isNSFW(providerComboBox.currentIndex)
            toolTipText: i18ndc("plasma_wallpaper_org.kde.potd", "@info:tooltip", "This wallpaper provider does not filter out images that may be sensitive or objectionable. Use caution if these images will be displayed in public.")
        }
    }

    QQC2.CheckBox {
        id: updateOverMeteredConnectionCheckBox

        checked: root.cfg_UpdateOverMeteredConnection === 1
        visible: backend.networkManagerQtAvailable
        text: i18ndc("plasma_wallpaper_org.kde.potd", "@option:check", "Update when using metered network connection")

        onToggled: {
            root.cfg_UpdateOverMeteredConnection = checked ? 1 : 0;
        }
    }

    QQC2.ComboBox {
        id: categoryComboBox
        visible: cfg_Provider === 'unsplash'
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox", "Category:")
        property var allSectionValue: '1065976'
        property var listModel: [
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "All"),
                'value': allSectionValue
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
            {
                'label': i18ndc("plasma_wallpaper_org.kde.potd", "@item:inlistbox", "Tabliss"),
                'value': '1053828'
            },
        ]
        model: listModel
        textRole: "label"
        onCurrentIndexChanged: {
            cfg_Category = model[currentIndex]["value"]
        }
        Component.onCompleted: {
            listModel = listModel.sort((a, b) => {
                // Sort items by name but keep the 'All' entry at the top
                if (b["value"] === allSectionValue) {
                   return 1;
                }
                if (a["value"] === allSectionValue) {
                   return -1;
                }
                return a["label"].localeCompare(b["label"]);
            })
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

    Kirigami.Separator {
        id: previewSeparator
        Kirigami.FormData.isSection: true
        visible: wallpaperPreview.visible
    }

    WallpaperPreview {
        id: wallpaperPreview
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label", "Today's picture:")
        backgroundColor: root.cfg_Color
        visible: !!wallpaper.configuration.Provider
    }

    Item {
        width: wallpaperPreview.implicitWidth
        height: wallpaperPreview.shadowOffset
    }

    Item {
        Kirigami.FormData.isSection: false
    }

    Kirigami.SelectableLabel {
        id: titleLabel
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label", "Title:")
        Layout.fillWidth: true
        Layout.maximumWidth: wallpaperPreview.implicitWidth * 1.5
        visible: wallpaperPreview.visible && backend.title.length > 0
        font.bold: true
        text: backend.title
        Accessible.name: titleLabel.Kirigami.FormData.label
    }

    Item {
        Kirigami.FormData.isSection: false
    }

    Kirigami.SelectableLabel {
        id: authorLabel
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.potd", "@label", "Author:")
        Layout.fillWidth: true
        Layout.maximumWidth: wallpaperPreview.implicitWidth * 1.5
        visible: wallpaperPreview.visible && backend.author.length > 0
        text: backend.author
        Accessible.name: authorLabel.Kirigami.FormData.label
    }

    Kirigami.InlineMessage {
        id: saveMessage

        Kirigami.FormData.isSection: true
        anchors.left: previewSeparator.left
        anchors.right: previewSeparator.right

        showCloseButton: true

        actions: [
            Kirigami.Action {
                icon.name: "document-open-folder"
                text: i18ndc("plasma_wallpaper_org.kde.potd", "@action:button", "Open Containing Folder")
                visible: backend.saveStatus === Global.Successful
                onTriggered: Qt.openUrlExternally(backend.savedFolder)

                Accessible.description: i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis for a button", "Open the destination folder where the wallpaper image was saved.")
            }
        ]

        onLinkActivated: Qt.openUrlExternally(backend.savedUrl)

        Connections {
            target: backend

            function onSaveStatusChanged() {
                switch (backend.saveStatus) {
                case Global.Successful:
                    saveMessage.text = backend.saveStatusMessage;
                    saveMessage.type = Kirigami.MessageType.Positive;
                    break;
                case Global.Failed:
                    saveMessage.text = backend.saveStatusMessage;
                    saveMessage.type = Kirigami.MessageType.Error;
                    break;
                default:
                    return;
                }

                saveMessage.visible = true;
            }
        }
    }
}
