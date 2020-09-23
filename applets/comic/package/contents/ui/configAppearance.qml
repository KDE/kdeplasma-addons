/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami


Kirigami.FormLayout {
    id: root
    anchors.left: parent.left
    anchors.right: parent.right

    signal configurationChanged

    function saveConfig() {
        plasmoid.nativeInterface.arrowsOnHover = showArrowsOnOver.checked;
        plasmoid.nativeInterface.showComicTitle = showComicTitle.checked;
        plasmoid.nativeInterface.showComicIdentifier = showIdentifier.checked;
        plasmoid.nativeInterface.showComicAuthor = showAuthor.checked;
        plasmoid.nativeInterface.showComicUrl = showUrl.checked;

        plasmoid.nativeInterface.saveConfig();
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        showArrowsOnOver.checked = plasmoid.nativeInterface.arrowsOnHover;
        showComicTitle.checked = plasmoid.nativeInterface.showComicTitle;
        showIdentifier.checked = plasmoid.nativeInterface.showComicIdentifier;
        showAuthor.checked = plasmoid.nativeInterface.showComicAuthor;
        showUrl.checked = plasmoid.nativeInterface.showComicUrl;
    }

    Controls.CheckBox {
        id: showComicTitle
        Kirigami.FormData.label: i18nc ("Heading for showing various elements of a comic", "Show:")
        text: i18nc("@option:check", "Comic title")
        onCheckedChanged: root.configurationChanged();
    }

    Controls.CheckBox {
        id: showIdentifier
        text: i18nc("@option:check", "Comic identifier")
        onCheckedChanged: root.configurationChanged();
    }

    Controls.CheckBox {
        id: showAuthor
        text: i18nc("@option:check", "Comic author")
        onCheckedChanged: root.configurationChanged();
    }

    Controls.CheckBox {
        id: showUrl
        text: i18nc("@option:check", "Comic URL")
        onCheckedChanged: root.configurationChanged();
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    Controls.RadioButton {
        id: alwaysShowArrows
        Kirigami.FormData.label: i18n ("Show navigation buttons:")
        text: i18nc("@option:check", "Always")
        checked: !showArrowsOnOver.checked
        onCheckedChanged: root.configurationChanged();
    }

    Controls.RadioButton {
        id: showArrowsOnOver
        text: i18nc("@option:check", "Only on hover")
        onCheckedChanged: root.configurationChanged();
    }
}
