/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property alias cfg_arrowsOnHover: showArrowsOnOver.checked
    property alias cfg_showComicTitle: showComicTitle.checked;
    property alias cfg_showComicIdentifier: showIdentifier.checked;
    property alias cfg_showComicAuthor: showAuthor.checked;
    property alias cfg_showComicUrl: showUrl.checked;

    Kirigami.FormLayout {
        Controls.CheckBox {
            id: showComicTitle
            Kirigami.FormData.label: i18nc ("Heading for showing various elements of a comic", "Show:")
            text: i18nc("@option:check", "Comic title")
        }

        Controls.CheckBox {
            id: showIdentifier
            text: i18nc("@option:check", "Comic identifier")
        }

        Controls.CheckBox {
            id: showAuthor
            text: i18nc("@option:check", "Comic author")
        }

        Controls.CheckBox {
            id: showUrl
            text: i18nc("@option:check", "Comic URL")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        Controls.RadioButton {
            id: alwaysShowArrows
            Kirigami.FormData.label: i18n ("Show navigation buttons:")
            text: i18nc("@option:check", "Always")
            checked: !showArrowsOnOver.checked
        }

        Controls.RadioButton {
            id: showArrowsOnOver
            text: i18nc("@option:check", "Only on hover")
        }
    }
}
