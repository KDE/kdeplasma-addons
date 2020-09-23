/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
