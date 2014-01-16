/***************************************************************************
 *   Copyright (C) 2013 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0 as QtLayouts
import org.kde.plasma.core 2.0 as PlasmaCore

Item
{
    id: configRoot
    property string cfg_text
    property string cfg_image
    property alias cfg_historySize: historySpin.value

    onCfg_textChanged: textServicesCombo.currentIndex = textServicesCombo.find(configRoot.cfg_text);
    onCfg_imageChanged: imageServicesCombo.currentIndex = imageServicesCombo.find(configRoot.cfg_image);

    QtLayouts.GridLayout {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        columns: 2

        PlasmaCore.DataSource {
            id: shareDataSource
            engine: "org.kde.plasma.dataengine.share"
            connectedSources: ["Mimetypes"]
        }

        QtControls.Label { text: i18n("Text Service:") }
        QtControls.ComboBox {
            id: textServicesCombo
            enabled: shareDataSource.data["Mimetypes"] != null
            model: shareDataSource.data["Mimetypes"]["text/*"]
            onCurrentTextChanged: configRoot.cfg_text = currentText
        }

        QtControls.Label { text: i18n("Image Service:") }
        QtControls.ComboBox {
            id: imageServicesCombo
            enabled: shareDataSource.data["Mimetypes"] != null
            model: shareDataSource.data["Mimetypes"]["image/*"]
            onCurrentTextChanged: configRoot.cfg_image = currentText
        }

        QtControls.Label { text: i18n("History Size:") }
        QtControls.SpinBox {
            id: historySpin
            value: 3
        }
    }
}
