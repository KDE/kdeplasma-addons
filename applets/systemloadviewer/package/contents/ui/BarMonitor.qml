/*
 * SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: barMonitor

    property var colors
    property var proportions

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle {
        id: barBorder
        anchors.fill: parent
        color: "transparent"
        radius: 3
        opacity: .4
        border {
            color: PlasmaCore.ColorScope.textColor
            width: 1
        }
    }

    Repeater {
        id: barRepeater
        model: proportions.length
        ConditionallyRoundedRectangle {
            color: barMonitor.colors[index]
            height: (barBorder.height - barBorder.border.width) * barMonitor.proportions[index]
            borderRectangle: barBorder
            // Bars should always be below border
            z: -1
            anchors {
                bottom: index == 0 ? barBorder.bottom : barRepeater.itemAt(index-1).top
                bottomMargin: index == 0 ? barBorder.border.width : 0
            }
        }
    }
}
