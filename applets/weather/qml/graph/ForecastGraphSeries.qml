/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtGraphs
import org.kde.kirigami as Kirigami

SplineSeries {
    id: root

    required property color seriesColor
    required property int xSection
    required property int ySection
    required property var model

    required property int selectedIndex
    required property bool graphHovered

    signal pointSelected(real x, real y)

    width: 3
    color: seriesColor

    pointDelegate: Item {
        id: delegate

        property real pointValueX
        property real pointValueY
        property int pointIndex

        Rectangle {
            anchors.centerIn: parent
            width: Kirigami.Units.gridUnit * 0.5
            height: width
            radius: width / 2
            color: root.seriesColor

            visible: root.selectedIndex === delegate.pointIndex && root.graphHovered

            onVisibleChanged: {
                if (visible) {
                    root.pointSelected(delegate.pointValueX, delegate.pointValueY);
                }
            }
        }
    }

    XYModelMapper {
        orientation: Qt.Horizontal
        xSection: root.xSection
        ySection: root.ySection
        model: root.model
        series: root
    }
}
