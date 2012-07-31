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
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: root

    property int minimumWidth: 364
    property int minimumHeight: 265

    anchors.fill: parent

    PlasmaCore.Theme {
        id: theme
    }

    TopPanel {
        id: panel
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }

    PlasmaComponents.TabBar {
        id: tabBar
        anchors {
            top: panel.bottom
            topMargin: 5
            horizontalCenter: parent.horizontalCenter
        }
        width: 160 // XXX
        height: 30 // XXX

        PlasmaComponents.TabButton {
            text: i18n("%1 Days").arg(3)
        }
        PlasmaComponents.TabButton {
            text: i18n("Details")
        }
    }

    FiveDaysView {
        id: fiveDaysView
        anchors {
            top: tabBar.bottom
            bottom: courtesyLabel.top
            left: parent.left
            right: parent.right
            topMargin: 14
        }
    }

    Text {
        id: courtesyLabel
        anchors {
            bottom: parent.bottom
            right: parent.right
        }
        font {
            pointSize: theme.smallestFont.pointSize
            underline: true
        }
        text: "Supported by backstage.bbc.co.uk"
    }
}
