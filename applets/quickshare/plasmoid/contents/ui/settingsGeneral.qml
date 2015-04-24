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
import QtQuick.Layouts 1.0 as QtLayouts
import org.kde.plasma.core 2.0 as PlasmaCore

Item
{
    id: configRoot
    property alias cfg_historySize: historySpin.value
    property alias cfg_copyAutomatically: copyAutomatically.checked

    QtLayouts.GridLayout {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        columns: 2

        QtControls.Label { text: i18n("History Size:") }
        QtControls.SpinBox {
            id: historySpin
            value: 3
        }

        QtControls.Label { text: i18n("Copy Automatically:") }
        QtControls.CheckBox {
            id: copyAutomatically
        }
    }
}
