/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as QQC2
import org.kde.kirigami 2.5 as Kirigami

import "logic.js" as Logic

Kirigami.FormLayout {
    id: appearancePage
    anchors.left: parent.left
    anchors.right: parent.right

    property alias cfg_autoClipboard: autoClipboardCheckBox.checked
    property alias cfg_compactPreviewCount: compactPreviewCountSpinBox.value
    property string cfg_defaultFormat
    property bool cfg_pickOnActivate

    QQC2.ComboBox {
        id: defaultFormatCombo
        Kirigami.FormData.label: i18nc("@label:listbox", "Default color format:")
        model: Logic.formats
        currentIndex: defaultFormatCombo.model.indexOf(cfg_defaultFormat)
        onActivated: cfg_defaultFormat = model[index]
    }

    QQC2.CheckBox {
        id: autoClipboardCheckBox
        text: i18nc("@option:check", "Automatically copy color to clipboard")
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.RadioButton {
        Kirigami.FormData.label: i18nc("@label", "When pressing the keyboard shortcut:")
        text: i18nc("@option:radio", "Pick a color")
        checked: cfg_pickOnActivate
        onCheckedChanged: cfg_pickOnActivate = checked
    }

    QQC2.RadioButton {
        text: i18nc("@option:radio", "Show history")
        checked: !cfg_pickOnActivate
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.SpinBox {
        id: compactPreviewCountSpinBox
        Kirigami.FormData.label: i18nc("@label", "Preview count:")
        from: 0
        to: 9 /* root.maxColorCount */
    }
}
