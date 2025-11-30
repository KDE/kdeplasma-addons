/*
 *  SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.kirigami as Kirigami

ListDelegate {
    id: item

    activeFocusOnTab: true

    iconItem: Kirigami.Icon {
        anchors.fill: parent
        source: item.icon.name
    }
}
