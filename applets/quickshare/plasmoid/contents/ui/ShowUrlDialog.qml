/************************************************************************************
 * Copyright (C) 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>                *
 *                                                                                  *
 * This program is free software; you can redistribute it and/or                    *
 * modify it under the terms of the GNU General Public License                      *
 * as published by the Free Software Foundation; either version 2                   *
 * of the License, or (at your option) any later version.                           *
 *                                                                                  *
 * This program is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 * GNU General Public License for more details.                                     *
 *                                                                                  *
 * You should have received a copy of the GNU General Public License                *
 * along with this program; if not, write to the Free Software                      *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 ************************************************************************************/

import QtQuick 2.2
import QtQuick.Layouts 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.purpose 1.0 as Purpose

PlasmaCore.Dialog
{
    id: window
    flags: Qt.WindowStaysOnTopHint
    property string url: ""
    signal copyUrl()

    hideOnWindowDeactivate: true
    mainItem: ColumnLayout {
        height: units.gridUnit * 16
        width: units.gridUnit * 16
        RowLayout {
            PlasmaComponents3.TextField {
                id: field
                Layout.fillWidth: true
                readOnly: true
                focus: true
                text: window.url
            }
            PlasmaComponents3.Button {
                icon.name: "edit-copy"
                onClicked: {
                    field.selectAll()
                    window.copyUrl()
                }
            }
        }
        PlasmaComponents3.Label {
            text: i18n("The URL was just shared")
        }
        Item {
            Layout.fillHeight: true
        }
        PlasmaComponents3.CheckBox {
            text: i18nc("@option:check", "Don't show this dialog, copy automatically.")
            checked: plasmoid.configuration.copyAutomatically
            onClicked: {
                plasmoid.configuration.copyAutomatically = checked
            }
        }
        PlasmaComponents3.Button {
            Layout.alignment: Qt.AlignRight
            text: i18nc("@action:button", "Close")
            onClicked: {
                window.visible = false
            }
        }
    }
}
