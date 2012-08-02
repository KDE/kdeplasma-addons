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

    height: 56 // XXX
    imagePath: "widgets/frame"
    prefix: "plain"
    visible: model.location.length > 0

    QtExtraComponents.QIconItem {
        icon: model.conditionIcon
        height: parent.height // XXX
        width: height
    }

    Text {
        id: locationLabel
        anchors {
            top: parent.top
            left: parent.left
            right: tempLabel.visible ? forecastTempsLabel.left : parent.right
            topMargin: 5
            leftMargin: 76
        }
        font.bold: true
        text: model.location
        elide: Text.ElideRight

        Component.onCompleted: font.pointSize = Math.floor(font.pointSize * 1.4);
    }

    Text {
        id: conditionLabel
        anchors {
            top: locationLabel.bottom
            left: locationLabel.left
            topMargin: 5
        }
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
        text: model.temp
    }

    Text{
        id: forecastTempsLabel
        anchors {
            right: tempLabel.right
            top: conditionLabel.top
        }
        font.pointSize: theme.smallestFont.pointSize
        text: model.forecastTemps
    }
}
