/*
 *  SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15

import org.kde.plasma.core 2.1 as PlasmaCore

ListDelegate {
    id: item

    activeFocusOnTab: true

    iconItem: PlasmaCore.IconItem {
        anchors.fill: parent
        source: item.icon.name
    }
}
