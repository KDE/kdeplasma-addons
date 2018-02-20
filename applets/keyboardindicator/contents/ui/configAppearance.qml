/*
*  Copyright 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
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

import QtQuick 2.2
import QtQml.Models 2.2
import QtQuick.Controls 1.1 as Controls
import QtQuick.Layouts 1.1 as Layouts

Controls.GroupBox
{
    signal configurationChanged

    function saveConfig() {
        plasmoid.nativeInterface.key = keysModel.get(keysCombo.currentIndex).name

        plasmoid.nativeInterface.saveConfig();
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        for(var i=0; i<keysModel.count; ++i) {
            if (keysModel.get(i).name === plasmoid.nativeInterface.key) {
                keysCombo.currentIndex = i
            }
        }
    }

    title: i18n("Appearance")
    flat: true

    Layouts.ColumnLayout {
        Controls.ComboBox {
            id: keysCombo
            textRole: "name"
            model: ObjectModel {
                id: keysModel
                QtObject {
                    readonly property string name: "Caps Lock"
                    readonly property string text: i18n("Caps Lock")
                }
                QtObject {
                    readonly property string name: "Num Lock"
                    readonly property string text: i18n("Num Lock")
                }
            }
        }
    }
}
