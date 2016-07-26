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
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0

import "../code/logic.js" as Logic

ColumnLayout {
    id: appearancePage

    property alias cfg_autoClipboard: autoClipboardCheckBox.checked
    property string cfg_defaultFormat
    property bool cfg_pickOnActivate

    QtControls.CheckBox {
        id: autoClipboardCheckBox
        Layout.fillWidth: true
        text: i18n("Automatically copy color to clipboard")
    }

    GridLayout {
        Layout.fillWidth: false // Layout thinks it's smart whereas it's not
        Layout.alignment: Qt.AlignLeft
        columns: 2

        QtControls.Label {
            Layout.row: 0
            Layout.column: 0
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: i18n("Default color format:")
        }

        QtControls.ComboBox {
            id: defaultFormatCombo
            Layout.row: 0
            Layout.column: 1
            // ComboBox default sizing is totally off
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 12
            model: Logic.formats
            currentIndex: defaultFormatCombo.model.indexOf(cfg_defaultFormat)
            onActivated: cfg_defaultFormat = model[index]
        }

        QtControls.Label {
            Layout.row: 1
            Layout.column: 0
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: i18n("When pressing the keyboard shortcut:")
        }

        QtControls.ExclusiveGroup {
            id: activationExclusiveGroup
        }

        QtControls.RadioButton {
            Layout.row: 1
            Layout.column: 1
            text: i18n("Pick a color")
            exclusiveGroup: activationExclusiveGroup
            checked: cfg_pickOnActivate
            onCheckedChanged: cfg_pickOnActivate = checked
        }

        QtControls.RadioButton {
            Layout.row: 2
            Layout.column: 1
            text: i18n("Show history")
            exclusiveGroup: activationExclusiveGroup
            checked: !cfg_pickOnActivate
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
