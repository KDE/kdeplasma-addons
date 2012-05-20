/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root

    property int minimumWidth: bg.naturalSize.width
    property int minimumHeight: bg.naturalSize.height

    QtObject {
        id: resizeOpts
        property real widthRate: root.width / root.minimumWidth
        property real heightRate: root.height / root.minimumHeight
    }

    PlasmaCore.Svg {
        id: lcdSvg
        imagePath: "weatherstation/lcd"
    }

    PlasmaCore.Svg {
        id: iconsSvg
        imagePath: "weatherstation/weather_icons"
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        svg: lcdSvg
        elementId: "lcd_background"
    }

    PlasmaCore.SvgItem {
        id: bg
        anchors.fill: parent
        svg: lcdSvg
        elementId: "background"
    }

    // XXX: enable resizing
    // XXX: fix "moon", "sun" and "clouds" size
    PlasmaCore.SvgItem {
        anchors {
            top: parent.top
            topMargin: 20 // XXX: fix positioning
            horizontalCenter: parent.horizontalCenter
        }
        height: iconsSvg.size.height
        width: iconsSvg.size.width
        svg: iconsSvg
        elementId: "weather:snow_rain"
    }

    // XXX: enable resizing
    LCDDisplay {
        id: pressureDisplay
        anchors {
            right: parent.right
            bottom: humidityDisplay.top
            rightMargin: 24 // XXX: fix positioning
            bottomMargin: 30 // XXX: fix positioning
        }
        height: 24 // XXX: fix initial size
        number: "29.8"
    }

    // XXX: enable resizing
    LCDDisplay {
        id: humidityDisplay
        anchors {
            right: parent.right
            bottom: windWidget.top
            rightMargin: 12 // XXX: fix positioning
            bottomMargin: 16 // XXX: fix positioning
        }
        number: "79"
    }

    // XXX: enable resizing
    LCDDisplay {
        id: temperatureDisplay
        anchors {
            right: parent.right
            bottom: windWidget.top
            rightMargin: 101 // XXX: fix positioning
            bottomMargin: 16 // XXX: fix positioning
        }
        number: "12"
    }

    Wind {
        id: windWidget
        anchors {
            bottom: parent.bottom
            bottomMargin: 10 * resizeOpts.heightRate
            horizontalCenter: parent.horizontalCenter
        }
        width: implicitWidth * resizeOpts.widthRate
        height: implicitHeight * resizeOpts.heightRate
        direction: "NE"
        speed: "12.3"
        unit: "m/s"
    }

    Text {
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        font {
            family: "DejaVu Sans"
            pixelSize: 7 // XXX: resize
        }
        color: "#202020"
        text: "PROVIDER"
    }
}
