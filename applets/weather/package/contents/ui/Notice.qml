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

Column {
    property alias model: repeater.model
    property alias title: title.text

    anchors.left: parent.left
    anchors.right: parent.right

    Text {
        id: title
        font.bold: true
        color: theme.textColor
    }

    Repeater {
        id: repeater

        Text {
            font.underline: true
            color: theme.linkColor
            text: modelData.description

            MouseArea {
                anchors.fill: parent
                onClicked: weatherApplet.invokeBrowser(modelData.info);
            }
        }
    }
}
