/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
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

import QtQuick 2.1

import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

GridLayout {
    id: iconAndTextRoot

    property alias iconSource: icon.source
    property alias text: label.text
    property bool vertical: false // too bad we cannot make this an enum
    property int minimumIconSize: units.iconSizes.small

    readonly property int iconSize: iconAndTextRoot.vertical ? width : height

    columns: iconAndTextRoot.vertical ? 1 : 2
    rows: iconAndTextRoot.vertical ? 2 : 1

    columnSpacing: 0
    rowSpacing: 0

    PlasmaCore.IconItem {
        id: icon

        readonly property int implicitMinimumIconSize: Math.max(iconSize, minimumIconSize)
        // reset implicit size, so layout in free dimension does not stop at the default one
        implicitWidth: minimumIconSize
        implicitHeight: minimumIconSize

        Layout.fillWidth: iconAndTextRoot.vertical
        Layout.fillHeight: !iconAndTextRoot.vertical
        Layout.minimumWidth: iconAndTextRoot.vertical ? minimumIconSize : implicitMinimumIconSize
        Layout.minimumHeight: iconAndTextRoot.vertical ? implicitMinimumIconSize : minimumIconSize
    }

    Item {
        id: text

        Layout.fillWidth: iconAndTextRoot.vertical
        Layout.fillHeight: !iconAndTextRoot.vertical
        Layout.minimumWidth: iconAndTextRoot.vertical ? 0 : sizehelper.paintedWidth
        Layout.maximumWidth: iconAndTextRoot.vertical ? Infinity : Layout.minimumWidth

        Layout.minimumHeight: iconAndTextRoot.vertical ? sizehelper.paintedHeight : 0
        Layout.maximumHeight: iconAndTextRoot.vertical ? Layout.minimumHeight : Infinity

        Text {
            id: sizehelper

            font {
                family: label.font.family
                weight: label.font.weight
                italic: label.font.italic
                pixelSize: iconAndTextRoot.vertical ? theme.mSize(theme.defaultFont).height * 2 : 1024 // random "big enough" size - this is used as a max pixelSize by the fontSizeMode
            }
            minimumPixelSize: theme.mSize(theme.smallestFont).height
            fontSizeMode: iconAndTextRoot.vertical ? Text.HorizontalFit : Text.VerticalFit
            wrapMode: Text.NoWrap

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors {
                fill: parent
                leftMargin: units.smallSpacing
                rightMargin: units.smallSpacing
            }
            visible: false

            // pattern to reserve some constant space TODO: improve and take formatting/i18n into account
            text: "888.8° X"
        }

        PlasmaComponents.Label {
            id: label

            font {
                weight: Font.Normal
                pixelSize: 1024
                pointSize: 0 // we need to unset pointSize otherwise it breaks the Text.Fit size mode
            }
            minimumPixelSize: theme.mSize(theme.smallestFont).height
            fontSizeMode: Text.Fit
            wrapMode: Text.NoWrap

            height: 0
            width: 0
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors {
                fill: parent
                leftMargin: units.smallSpacing
                rightMargin: units.smallSpacing
            }
        }
    }
}
