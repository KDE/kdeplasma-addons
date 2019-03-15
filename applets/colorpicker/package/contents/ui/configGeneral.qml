/*
 * Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as QtControls
import org.kde.kirigami 2.5 as Kirigami

import "logic.js" as Logic

Kirigami.FormLayout {
    id: appearancePage
    anchors.left: parent.left
    anchors.right: parent.right

    property alias cfg_autoClipboard: autoClipboardCheckBox.checked
    property string cfg_defaultFormat
    property bool cfg_pickOnActivate

    QtControls.ComboBox {
        id: defaultFormatCombo
        Kirigami.FormData.label: i18nc("@label:listbox", "Default color format:")
        model: Logic.formats
        currentIndex: defaultFormatCombo.model.indexOf(cfg_defaultFormat)
        onActivated: cfg_defaultFormat = model[index]
    }

    QtControls.CheckBox {
        id: autoClipboardCheckBox
        text: i18nc("@option:check", "Automatically copy color to clipboard")
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QtControls.RadioButton {
        Kirigami.FormData.label: i18nc("@label", "When pressing the keyboard shortcut:")
        text: i18nc("@option:radio", "Pick a color")
        checked: cfg_pickOnActivate
        onCheckedChanged: cfg_pickOnActivate = checked
    }

    QtControls.RadioButton {
        text: i18nc("@option:radio", "Show history")
        checked: !cfg_pickOnActivate
    }
}
