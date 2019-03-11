/*
 *  Copyright 2014 Joseph Wenninger <jowenn@kde.org>
 *  Copyright 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 *
 *  Based on analog-clock configGeneral.qml:
 *  Copyright 2013 David Edmundson <davidedmundson@kde.org>
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

import QtQuick 2.0
import QtQuick.Controls 2.5 as QtControls
import QtQuick.Layouts 1.1
import org.kde.kquickcontrols 2.0 as KQuickControls
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: generalConfigPage
    anchors.left: parent.left
    anchors.right: parent.right

    property alias cfg_showGrid: showGridCheckBox.checked
    property alias cfg_showOffLeds: showOffLedsCheckBox.checked
    property alias cfg_showSeconds: showSecondsCheckBox.checked
    property alias cfg_showBcdFormat: showBcdFormatCheckBox.checked
    property alias cfg_useCustomColorForActive: useCustomColorForActiveCheckBox.checked
    property alias cfg_customColorForActive: customColorForActive.color
    property alias cfg_useCustomColorForInactive: useCustomColorForInactiveCheckBox.checked
    property alias cfg_customColorForInactive: customColorForInactive.color
    property alias cfg_useCustomColorForGrid: useCustomColorForGridCheckBox.checked
    property alias cfg_customColorForGrid: customColorForGrid.color

        QtControls.CheckBox {
            id: showGridCheckBox
            Kirigami.FormData.label:i18n("Display:")
            text: i18nc("@option:check", "Grid")
        }

        QtControls.CheckBox {
            id: showOffLedsCheckBox
            text: i18nc("@option:check", "Inactive LEDs")
        }

        QtControls.CheckBox {
            id: showSecondsCheckBox
            text: i18nc("@option:check", "Seconds")
        }

        QtControls.CheckBox {
            id: showBcdFormatCheckBox
            text: i18nc("@option:check", "In BCD format (decimal)")
        }


        Item {
            Kirigami.FormData.isSection:true
        }

        RowLayout {
            Kirigami.FormData.label:i18n("Use custom color for:")

            QtControls.CheckBox {
                id: useCustomColorForActiveCheckBox
                text: i18nc("@option:check", "Active LEDs")
            }

            KQuickControls.ColorButton {
                id: customColorForActive
                enabled: useCustomColorForActiveCheckBox.checked
            }
        }

        RowLayout {

            QtControls.CheckBox {
                id: useCustomColorForInactiveCheckBox
                text: i18nc("@option:check", "Inactive LEDs")
            }

            KQuickControls.ColorButton {
                id:customColorForInactive
                enabled: useCustomColorForInactiveCheckBox.checked
            }
        }

        RowLayout {

            QtControls.CheckBox {
                id: useCustomColorForGridCheckBox
                text: i18nc("@option:check", "Grid")
            }

            KQuickControls.ColorButton {
                id:customColorForGrid
                enabled: useCustomColorForGridCheckBox.checked
            }
        }
}
