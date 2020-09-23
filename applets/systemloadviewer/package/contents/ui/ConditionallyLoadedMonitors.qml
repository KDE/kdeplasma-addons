/*
 * SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    property var colors
    property var proportions

    Layout.fillWidth: true
    Layout.fillHeight: true

    Loader {
        id: loader
        active: visible
        anchors.fill: parent
        source: switch (plasmoid.configuration.monitorType) {
            default: case 0: "BarMonitor.qml"; break;
            case 1: "CircularMonitor.qml"; break;
            case 2: "CompactBarMonitor.qml"; break;
        }
        onLoaded: {
            loader.item.colors = Qt.binding(function() { return colors })
            loader.item.proportions = Qt.binding(function() { return proportions })
        }
    }
}
