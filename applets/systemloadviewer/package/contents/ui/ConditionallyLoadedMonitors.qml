/*
 * Copyright (C) 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
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
        }
        onLoaded: {
            loader.item.colors = Qt.binding(function() { return colors })
            loader.item.proportions = Qt.binding(function() { return proportions })
        }
    }
}
