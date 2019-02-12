/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
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

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    id: root

    property alias model: categoryRepeater.model
    readonly property bool hasContent: model && model.length > 0 && (model[0].length > 0 || model[1].length > 0)

    spacing: units.largeSpacing

    Repeater {
        id: categoryRepeater

        delegate: ColumnLayout {
            property var categoryData: modelData

            readonly property bool categoryHasNotices: categoryData.length > 0
            visible: categoryHasNotices

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

            PlasmaExtras.Heading {
                level: 4
                Layout.alignment: Qt.AlignHCenter

                text: index == 0 ? i18nc("@title:column weather warnings", "Warnings Issued") : i18nc("@title:column weather watches" ,"Watches Issued")
            }

            Repeater {
                id: repeater

                model: categoryData

                delegate: PlasmaComponents.Label {
                    font.underline: true
                    color: theme.linkColor

                    text: modelData.description

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            Qt.openUrlExternally(modelData.info);
                        }
                    }
                }
            }
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
