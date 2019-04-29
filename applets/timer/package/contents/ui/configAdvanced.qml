/*
 *  Copyright 2015 Bernhard Friedrich <friesoft@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: generalPage

    property alias cfg_runCommand: runCommand.checked
    property alias cfg_command: command.text

    RowLayout {
        Layout.fillWidth: true

        Kirigami.FormData.label: i18nc("@title:label", "After timer completes:")

        QQC2.CheckBox {
            id: runCommand
            text: i18nc("@option:check", "Execute command:")
            onClicked: {
                if (checked) {
                    command.forceActiveFocus();
                }
            }
        }

        QQC2.TextField {
            id: command
            Layout.fillWidth: true
            enabled: runCommand.checked
        }
    }
}

