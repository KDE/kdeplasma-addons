/*
*  SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 2.2 as Controls
import QtQuick.Layouts 1.1

Item
{
    id: root
    signal configurationChanged

    function saveConfig() {
        var names = []
        for(var i in layout.children) {
            var cur = layout.children[i]
            if (cur.checked)
                names.push(cur.name)
        }
        plasmoid.configuration.key = names
    }

    ColumnLayout {
        id: layout
        Controls.CheckBox {
            Layout.fillWidth: true
            readonly property string name: "Caps Lock"
            checked: plasmoid.configuration.key.indexOf(name) >= 0
            text: i18nc("@option:check", "Caps Lock")
            onCheckedChanged: root.configurationChanged()
        }
        Controls.CheckBox {
            Layout.fillWidth: true
            readonly property string name: "Num Lock"
            checked: plasmoid.configuration.key.indexOf(name) >= 0
            text: i18nc("@option:check", "Num Lock")
            onCheckedChanged: root.configurationChanged()
        }
    }
}
