/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.core 2.1 as PlasmaCore

MouseArea {
    id: abstractItem
    property string text
    property string itemId
    property int status     //PlasmaCore.Types.ItemStatus
    property bool active

    hoverEnabled: true
    drag.filterChildren: true
    acceptedButtons: Qt.RightButton
}

