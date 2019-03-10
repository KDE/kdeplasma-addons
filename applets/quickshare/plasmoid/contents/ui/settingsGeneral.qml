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
import QtQuick.Controls 2.5
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: configRoot
    anchors.left: parent.left
    anchors.right: parent.right

    property alias cfg_historySize: historySpin.value
    property alias cfg_copyAutomatically: copyAutomatically.checked

    SpinBox {
        id: historySpin
        Kirigami.FormData.label: i18nc("@label:spinbox", "History size:")
        value: 3
    }

    Item {
        Kirigami.FormData.isSection: false
    }

    CheckBox {
        id: copyAutomatically
        Kirigami.FormData.label: i18nc("@option:check", "Copy automatically:")
    }
}
