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
        property real wScaleFactor: root.width / root.minimumWidth
        property real hScaleFactor: root.height / root.minimumHeight
    }

    Connections {
        target: backend

        onTemperatureChanged: {
            temperatureDisplay.number = (temperature != "-" ? temperature : "");
            temperatureDisplay.superscript = unit;
        }

        onHumidityChanged: humidityDisplay.number = (humidity != "N/A" ? humidity : "");

        onProviderLabelChanged: providerLabel.text = label;
        onWeatherLabelChanged: weatherLabel.text = label;

        onPressureChanged: {
            condition.elementId = conditionId;
            pressureDisplay.number = pressure;
            pressureDisplay.superscript = unit;
            pressureDirection.elementId = direction;
        }

        onWindChanged: {
            windWidget.direction = direction;
            windWidget.speed = speed;
            windWidget.unit = unit;
        }
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
        visible: backend.useBackground
    }

    PlasmaCore.SvgItem {
        id: bg
        anchors.fill: parent
        svg: lcdSvg
        elementId: "background"
    }

    Text {
        id: weatherLabel
        anchors {
            top: parent.top
            left: parent.left
            topMargin: 12 * resizeOpts.hScaleFactor
            leftMargin: 5 * resizeOpts.wScaleFactor
        }
        font {
            family: "DejaVu Sans"
            pixelSize: Math.round(8 * resizeOpts.wScaleFactor)
        }
        color: "#202020"
        smooth: true
        text: i18n("CURRENT WEATHER")
    }

    PlasmaCore.SvgItem {
        id: condition
        anchors {
            top: parent.top
            topMargin: 26 * resizeOpts.hScaleFactor
            horizontalCenter: parent.horizontalCenter
        }
        height: naturalSize.height * resizeOpts.hScaleFactor
        width: naturalSize.width * resizeOpts.wScaleFactor
        visible: elementId != ""
        svg: iconsSvg
    }

    Text {
        id: pressureLabel
        anchors {
            top: parent.top
            left: weatherLabel.left
            topMargin: 127 * resizeOpts.hScaleFactor
        }
        font: weatherLabel.font
        color: weatherLabel.color
        smooth: true
        text: i18n("PRESSURE")
    }

    PlasmaCore.SvgItem {
        id: pressureDirection
        anchors {
            left: parent.left
            leftMargin: 84 * resizeOpts.wScaleFactor
            verticalCenter: pressureDisplay.verticalCenter
        }
        height: naturalSize.height * resizeOpts.hScaleFactor
        width: naturalSize.width * resizeOpts.wScaleFactor
        visible: elementId != ""
        svg: lcdSvg
    }

    LCDDisplay {
        id: pressureDisplay
        anchors {
            right: parent.right
            bottom: pressureLabel.bottom
            rightMargin: 24 * resizeOpts.wScaleFactor
        }
        height: 23 * resizeOpts.hScaleFactor
        superscriptFont: weatherLabel.font
    }

    Text {
        id: temperatureLabel
        anchors {
            top: parent.top
            left: weatherLabel.left
            topMargin: 152 * resizeOpts.hScaleFactor
        }
        font: weatherLabel.font
        color: weatherLabel.color
        smooth: true
        text: i18n("OUTDOOR TEMP")
    }

    LCDDisplay {
        id: temperatureDisplay
        anchors {
            right: parent.right
            bottom: windWidget.top
            rightMargin: 97 * resizeOpts.wScaleFactor
            bottomMargin: 16 * resizeOpts.hScaleFactor
        }
        height: implicitHeight * resizeOpts.hScaleFactor
        superscriptFont: weatherLabel.font
    }

    Text {
        id: humidityLabel
        anchors {
            top: temperatureLabel.top
            right: parent.right
            rightMargin: 5 * resizeOpts.wScaleFactor
        }
        font: weatherLabel.font
        color: weatherLabel.color
        smooth: true
        text: i18n("HUMIDITY")
    }

    LCDDisplay {
        id: humidityDisplay
        anchors {
            top: temperatureDisplay.top
            right: parent.right
            rightMargin: 12 * resizeOpts.wScaleFactor
        }
        height: temperatureDisplay.height
        superscript: "%"
        superscriptFont: weatherLabel.font
    }

    Text {
        id: windLabel
        anchors {
            top: temperatureLabel.top
            left: weatherLabel.left
            topMargin: 60 * resizeOpts.hScaleFactor
        }
        font: weatherLabel.font
        color: weatherLabel.color
        smooth: true
        text: i18n("WIND")
    }

    Wind {
        id: windWidget
        anchors {
            bottom: parent.bottom
            bottomMargin: 10 * resizeOpts.hScaleFactor
            horizontalCenter: parent.horizontalCenter
        }
        width: implicitWidth * resizeOpts.wScaleFactor
        height: implicitHeight * resizeOpts.hScaleFactor
    }

    Text {
        id: providerLabel
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        font {
            family: "DejaVu Sans"
            pixelSize: 7 * resizeOpts.hScaleFactor
        }
        color: weatherLabel.color
        smooth: true

        MouseArea {
            anchors.fill: parent
            onClicked: backend.clicked();
        }
    }
}
