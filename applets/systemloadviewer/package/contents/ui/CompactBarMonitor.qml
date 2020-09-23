/*
 * SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 * SPDX-FileCopyrightText: 2015 Joshua Worth <joshua@worth.id.au>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

Item {
    id: barMonitor

    property var colors
    property var proportions: []

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle {
        id: barBorder
        anchors.fill: parent
        opacity: 0
    }

    Repeater {
        id: barRepeater
        model: proportions.length
        Rectangle {
            color: barMonitor.colors[index]
            height: barBorder.height * barMonitor.proportions[index]
            width: barBorder.width
            anchors {
                bottom: index == 0 ? barBorder.bottom : barRepeater.itemAt(index-1).top
                bottomMargin: index == 0 ? barBorder.border.width : 0
            }

            LinearGradient {
                anchors.fill: parent
                end: Qt.point(width, 0)
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#60ffffff" }
                    GradientStop { position: 1.0; color: "transparent" }
                }
            }
        }
    }
}
