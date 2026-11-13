/*
 * SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_boldText: boldCheckBox.checked
    property alias cfg_italicText: italicCheckBox.checked

    property alias cfg_fuzzyness: fuzzyness.value

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group", "Font:")
                contentItem: QQC2.CheckBox {
                    id: boldCheckBox
                    text: i18nc("@option:check", "Bold text")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check", "Italic text")
                    id: italicCheckBox
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group", "Fuzzyness:")
                contentItem: ColumnLayout {
                    Kirigami.FormData.buddyFor: fuzzyness
                    QQC2.Slider {
                        Layout.fillWidth: true
                        id: fuzzyness
                        from: 1
                        to: 5
                        stepSize: 1
                        Kirigami.StyleHints.tickMarkStepSize: 1
                    }
                    RowLayout {
                        QQC2.Label {
                            text: i18nc("@item:inrange", "Accurate")
                            textFormat: Text.PlainText
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        QQC2.Label {
                            text: i18nc("@item:inrange", "Fuzzy")
                            textFormat: Text.PlainText
                        }
                    }
                }
            }
        }
    }
}
