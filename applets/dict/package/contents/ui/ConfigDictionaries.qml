/*
 *  Copyright 2017 David Faure <faure@kde.org>
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

import QtQuick 2.7
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.private.dict 1.0

ColumnLayout {
    id: root
    property string cfg_dictionary: ""

    DictionariesModel {
        id: dictionariesModel
    }

    Kirigami.Heading {
        Layout.fillWidth: true
        level: 2
        text: i18nc("@label:listbox", "Available dictionaries:")
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentWidth: listView.implicitWidth
        contentHeight: listView.implicitHeight
        Component.onCompleted: background.visible = true;

        ListView {
            id: listView
            model: dictionariesModel

            delegate: Control {

                background: Rectangle {
                    id: highlight
                    implicitWidth: root.width - units.largeSpacing
                    visible: model.id == root.cfg_dictionary
                    color: Kirigami.Theme.highlightColor
                }

                contentItem: Label {
                    id: pathText
                    Layout.fillWidth: true
                    text: model.description
                    color: model.id == root.cfg_dictionary ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.cfg_dictionary = model.id
                }
            }
        }
    }
}
