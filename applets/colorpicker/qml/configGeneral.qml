/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

import "Logic.js" as Logic

KCM.SimpleKCM {
    id: root
    property alias cfg_autoClipboard: autoClipboardCheckBox.checked
    property alias cfg_compactPreviewCount: compactPreviewCountSpinBox.value
    property string cfg_defaultFormat
    property bool cfg_pickOnActivate

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Default color format:")
                contentItem: QQC2.ComboBox {
                    id: defaultFormatCombo
                    model: Logic.formats
                    currentIndex: model.indexOf(root.cfg_defaultFormat)
                    onActivated: index => {
                        root.cfg_defaultFormat = model[index];
                    }
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: autoClipboardCheckBox
                    text: i18nc("@option:check", "Automatically copy color to clipboard")
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label", "When pressing the keyboard shortcut:")
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "Pick a color")
                    checked: root.cfg_pickOnActivate
                    onToggled: root.cfg_pickOnActivate = checked
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "Show history")
                    checked: !root.cfg_pickOnActivate
                    onToggled: root.cfg_pickOnActivate = !checked
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label", "Preview count:")
                contentItem: QQC2.SpinBox {
                    id: compactPreviewCountSpinBox
                    from: 0
                    to: 9 /* root.maxColorCount */
                }
            }
        }
    }
}
