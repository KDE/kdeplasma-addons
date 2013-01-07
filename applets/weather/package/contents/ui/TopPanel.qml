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
import org.kde.qtextracomponents 0.1 as QtExtraComponents

PlasmaCore.FrameSvgItem {
    property variant model

    imagePath: "widgets/frame"
    prefix: "plain"
    visible: model.location.length > 0

    QtExtraComponents.QIconItem {
        icon: model.conditionIcon
        height: parent.height
        width: height
    }

    Text {
        id: locationLabel
        anchors {
            top: parent.top
            left: parent.left
            right: tempLabel.visible ? forecastTempsLabel.left : parent.right
            topMargin: 5
            leftMargin: parent.width * 0.21
        }
        font.bold: true
        color: theme.textColor
        text: model.location
        elide: Text.ElideRight

        Component.onCompleted: font.pointSize = Math.floor(font.pointSize * 1.4);
    }

    Text {
        id: conditionLabel
        anchors {
            top: parent.top
            left: locationLabel.left
            topMargin: parent.height * 0.6
        }
        color: theme.textColor
        text: model.conditions
    }

    Text {
        id: tempLabel
        anchors {
            right: parent.right
            top: locationLabel.top
            rightMargin: 5
        }
        font: locationLabel.font
        color: theme.textColor
        text: model.temp
    }

    Text{
        id: forecastTempsLabel
        anchors {
            right: tempLabel.right
            top: conditionLabel.top
        }
        font.pointSize: theme.smallestFont.pointSize
        color: theme.textColor
        text: model.forecastTemps
    }
}
