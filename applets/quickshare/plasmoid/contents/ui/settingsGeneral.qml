/***************************************************************************
 *   Copyright (C) 2013 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0

ColumnLayout {
    id: configRoot
    property alias cfg_historySize: historySpin.value
    property alias cfg_copyAutomatically: copyAutomatically.checked

    GridLayout {
        columns: 2

        QtControls.Label {
            Layout.row: 0
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:spinbox", "History size:")
        }
        QtControls.SpinBox {
            id: historySpin
            Layout.row: 0
            Layout.column: 1
            value: 3
        }

        QtControls.Label {
            Layout.row: 1
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@option:check", "Copy automatically:")
        }
        QtControls.CheckBox {
            Layout.row: 1
            Layout.column: 1
            id: copyAutomatically
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
