/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts as Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property alias cfg_arrowsOnHover: showArrowsOnOver.checked
    property alias cfg_showComicTitle: showComicTitle.checked;
    property alias cfg_showComicIdentifier: showIdentifier.checked;
    property alias cfg_showComicAuthor: showAuthor.checked;
    property alias cfg_showComicUrl: showUrl.checked;
    property alias cfg_middleClick: middleClickCheckBox.checked;

    Kirigami.FormLayout {
        Controls.CheckBox {
            id: showComicTitle
            Kirigami.FormData.label: i18nc ("@title:group form label for checkboxes", "Show above image:")  // qmllint disable unqualified
            text: i18nc("@option:check show above image: ", "Title")  // qmllint disable unqualified
        }

        Controls.CheckBox {
            id: showAuthor
            text: i18nc("@option:check show above image: ", "Author")  // qmllint disable unqualified
        }

        Controls.CheckBox {
            id: showIdentifier
            Kirigami.FormData.label: i18nc ("@title:group form label for checkboxes", "Show below image:")  // qmllint disable unqualified
            text: i18nc("@option:check show below image", "Identifier")  // qmllint disable unqualified
        }

        Controls.CheckBox {
            id: showUrl
            text: i18nc("@option:check show below image", "Website")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        Controls.RadioButton {
            id: alwaysShowArrows
            Kirigami.FormData.label: i18nc("@label:group form label for radiobuttons", "Navigation buttons:")  // qmllint disable unqualified
            text: i18nc("@option:radio navigation buttons are…", "Always visible at the edges")  // qmllint disable unqualified
            checked: !showArrowsOnOver.checked
        }

        Controls.RadioButton {
            id: showArrowsOnOver
            text: i18nc("@option:radio navigation buttons…", "Appear centrally when hovering")  // qmllint disable unqualified
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        Controls.CheckBox {
            id: middleClickCheckBox
            Kirigami.FormData.label: i18nc("@label for checkbox, completes sentence … opens full-size view", "Middle-clicking image:")  // qmllint disable unqualified
            text: i18nc("@option:check completes sentence Middle-clicking image…", "Opens full-size view")  // qmllint disable unqualified
        }
    }
}
