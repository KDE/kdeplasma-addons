/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property alias cfg_arrowsOnHover: showArrowsOnOver.checked
    property alias cfg_showComicTitle: showComicTitle.checked;
    property alias cfg_showComicIdentifier: showIdentifier.checked;
    property alias cfg_showComicAuthor: showAuthor.checked;
    property alias cfg_showComicUrl: showUrl.checked;
    property alias cfg_middleClick: middleClickCheckBox.checked;

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group form label for checkboxes", "Show above image:")
                contentItem: Controls.CheckBox {
                    id: showComicTitle
                    text: i18nc("@option:check show above image: ", "Title")
                }
            }
            Kirigami.FormEntry {
                contentItem: Controls.CheckBox {
                    id: showAuthor
                    text: i18nc("@option:check show above image: ", "Author")
                }
            }
            Kirigami.FormEntry {
                title: i18nc ("@title:group form label for checkboxes", "Show below image:")
                contentItem: Controls.CheckBox {
                    id: showIdentifier
                    text: i18nc("@option:check show below image", "Identifier")
                }
            }
            Kirigami.FormEntry {
                contentItem: Controls.CheckBox {
                    id: showUrl
                    text: i18nc("@option:check show below image", "Website")
                }
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:group form label for radiobuttons", "Navigation buttons:")
                contentItem: Controls.RadioButton {
                    id: alwaysShowArrows
                    text: i18nc("@option:radio navigation buttons are…", "Always visible at the edges")
                    checked: !showArrowsOnOver.checked
                }
            }
            Kirigami.FormEntry {
                contentItem: Controls.RadioButton {
                    id: showArrowsOnOver
                    text: i18nc("@option:radio navigation buttons…", "Appear centrally when hovering")
                }
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label for checkbox, completes sentence … opens full-size view", "Middle-clicking image:")
                contentItem: Controls.CheckBox {
                    id: middleClickCheckBox
                    text: i18nc("@option:check completes sentence Middle-clicking image…", "Opens full-size view")
                }
            }
        }
    }
}
