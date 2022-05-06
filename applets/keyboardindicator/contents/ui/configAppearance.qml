/*
*  SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 2.2 as Controls

import org.kde.kirigami 2.19 as Kirigami

Kirigami.FormLayout {
    id: layout
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

    Controls.CheckBox {
        Kirigami.FormData.label: i18nc("@label show keyboard indicator when Caps Lock or Num Lock is activated", "Show when activated:")
        readonly property string name: "Caps Lock"
        checked: plasmoid.configuration.key.indexOf(name) >= 0
        text: i18nc("@option:check", "Caps Lock")
        onToggled: layout.configurationChanged()
    }

    Controls.CheckBox {
        readonly property string name: "Num Lock"
        checked: plasmoid.configuration.key.indexOf(name) >= 0
        text: i18nc("@option:check", "Num Lock")
        onToggled: layout.configurationChanged()
    }
}
