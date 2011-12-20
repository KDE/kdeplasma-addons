/*   vim:set foldenable foldmethod=marker:
 *
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.qtextracomponents 0.1

Item {
    id: main

    property int itemSize: 32

    width: parent.width
    height: itemSize + 16

    /* property declarations --------------------------{{{ */
    property alias title: textTitle.text
    property alias description: textDescription.text
    property alias icon: imageIcon.icon
    /* }}} */

    /* signal declarations ----------------------------{{{ */
    /* }}} */

    /* JavaScript functions ---------------------------{{{ */
    /* }}} */

    /* object properties ------------------------------{{{ */
    /* }}} */

    /* child objects ----------------------------------{{{ */

        QIconItem {
            id: imageIcon
            anchors.verticalCenter: parent.verticalCenter
            x:      8
            width:  main.itemSize
            height: main.itemSize
            // icon: QIcon(main.icon)
        }

        Text {
            id: textTitle

            anchors {
                left:   imageIcon.right
                top:    parent.top
                bottom: (textDescription.text == "") ? parent.bottom : parent.verticalCenter
                right:  parent.right

                leftMargin: 8
            }

            verticalAlignment: (textDescription.text == "") ? Text.AlignVCenter : Text.AlignBottom
            elide: Text.ElideRight
        }

        Text {
            id: textDescription

            anchors {
                left:   imageIcon.right
                top:    parent.verticalCenter
                bottom: parent.bottom
                right:  parent.right

                leftMargin: 8
            }

            opacity: .6
            elide: Text.ElideRight
            visible: (text != "")
        }

        MouseArea {
            anchors.fill: parent

            onClicked: {
                print("clicked")
                var service = recommendationsModel.serviceForSource(DataEngineSource)
                var operation = service.operationDescription("executeAction")
                operation.Action = ""
                service.startOperationCall(operation)
            }

            preventStealing: true
        }
    /* }}} */

    /* states -----------------------------------------{{{ */
    /* }}} */

    /* transitions ------------------------------------{{{ */
    /* }}} */
}

