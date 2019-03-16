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
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami


Kirigami.FormLayout {
    id: root
    anchors.left: parent.left
    anchors.right: parent.right

    signal configurationChanged

    function saveConfig() {
        plasmoid.nativeInterface.showErrorPicture = showErrorPicture.checked;
        plasmoid.nativeInterface.maxComicLimit = maxComicLimit.value;

        plasmoid.nativeInterface.saveConfig();
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        showErrorPicture.checked = plasmoid.nativeInterface.showErrorPicture;
        maxComicLimit.value = plasmoid.nativeInterface.maxComicLimit;
    }

    Layouts.RowLayout {
        Kirigami.FormData.label: i18nc("@label:spinbox", "Comic cache:")

        Controls.SpinBox {
            id: maxComicLimit
            stepSize: 1
            onValueChanged: root.configurationChanged();
        }

        Controls.Label {
            text: i18ncp("@item:valuesuffix spacing to number + unit", "strip per comic", "strips per comic")
        }
    }

    Controls.CheckBox {
        id: showErrorPicture
        text: i18nc("@option:check", "Display error when downloading comic fails")
        onCheckedChanged: root.configurationChanged();
    }
}
