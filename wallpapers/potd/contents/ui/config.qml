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
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Dialogs 1.1 as QtDialogs
import org.kde.plasma.core 2.0 as PlasmaCore

Column {
    id: root
    property string cfg_Provider
    property int cfg_FillMode
    property alias cfg_Color: colorDialog.color
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
            text: i18nd("plasma_applet_org.kde.potd", "Providers:")
            anchors.verticalCenter: providerComboBox.verticalCenter
        }
        QtControls.ComboBox {
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
            id: positionLabel
            width: formAlignment - units.largeSpacing
            anchors {
                verticalCenter: resizeComboBox.verticalCenter
            }
            text: i18nd("plasma_applet_org.kde.potd", "Positioning:")
            horizontalAlignment: Text.AlignRight
        }
        QtControls.ComboBox {
            id: resizeComboBox
            property int textLength: 24
            width: theme.mSize(theme.defaultFont).width * textLength
            model: [
                        {
                            'label': i18nd("plasma_applet_org.kde.potd", "Scaled and Cropped"),
                            'fillMode': Image.PreserveAspectCrop
                        },
                        {
                            'label': i18nd("plasma_applet_org.kde.potd","Scaled"),
                            'fillMode': Image.Stretch
                        },
                        {
                            'label': i18nd("plasma_applet_org.kde.potd","Scaled, Keep Proportions"),
                            'fillMode': Image.PreserveAspectFit
                        },
                        {
                            'label': i18nd("plasma_applet_org.kde.potd", "Centered"),
                            'fillMode': Image.Pad
                        },
                        {
                            'label': i18nd("plasma_applet_org.kde.potd","Tiled"),
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

    QtDialogs.ColorDialog {
        id: colorDialog
        modality: Qt.WindowModal
        showAlphaChannel: false
        title: i18nd("plasma_applet_org.kde.potd", "Select Background Color")
    }

    Row {
        id: colorRow
        spacing: units.largeSpacing / 2
        QtControls.Label {
            width: formAlignment - units.largeSpacing
            anchors.verticalCenter: colorButton.verticalCenter
            horizontalAlignment: Text.AlignRight
            text: i18nd("plasma_applet_org.kde.potd", "Background Color:")
        }
        QtControls.Button {
            id: colorButton
            width: units.gridUnit * 3
            text: " " // needed to it gets a proper height...
            onClicked: colorDialog.open()

            Rectangle {
                id: colorRect
                anchors.centerIn: parent
                width: parent.width - 2 * units.smallSpacing
                height: theme.mSize(theme.defaultFont).height
                color: colorDialog.color
            }
        }
    }
}
