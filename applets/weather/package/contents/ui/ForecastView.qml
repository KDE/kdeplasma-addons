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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {
    id: root

    property alias model: repeater.model
    readonly property int preferredIconSize: units.iconSizes.large
    readonly property bool hasContent: model && model.length > 0

    spacing: units.smallSpacing

    Component {
        id: timeDelegate

        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            font.bold: true
            // still needed? was in old code
            opacity: (text.indexOf("ight") !== -1 || text.indexOf("nite") !== -1) ? 0.6 : 1
            horizontalAlignment: Text.AlignHCenter

            text: {
                var time = cellData;
                // still needed? was in old code
                if (time.indexOf("nt") !== -1) {
                    time = time.replace(" nt", "");
                }
                return time;
            }
        }
    }

    Component {
        id: temperatureDelegate

        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            horizontalAlignment: Text.AlignHCenter

            text: cellData
        }
    }

    Component {
        id: iconDelegate

        PlasmaCore.IconItem {
            Layout.fillWidth: true
            Layout.preferredHeight: preferredIconSize
            Layout.preferredWidth: preferredIconSize

            PlasmaCore.ToolTipArea {
                id: iconToolTip

                anchors.fill: parent
            }

            Component.onCompleted: {
                var values = cellData.split("|");
                source = values[0];
                iconToolTip.mainText = values[1];
            }
        }
    }

    // simulate a table, by creating rowlayouts whose items all have the same width
    Repeater {
        id: repeater

        delegate: Loader {
            readonly property int rowIndex: index
            readonly property var rowData: modelData

            Layout.fillWidth: item.Layout.fillWidth
            Layout.fillHeight: item.Layout.fillHeight
            Layout.minimumWidth: item.Layout.minimumWidth
            Layout.minimumHeight: item.Layout.minimumHeight
            Layout.preferredWidth: item.Layout.preferredWidth
            Layout.preferredHeight: item.Layout.preferredHeight

            sourceComponent: RowLayout {
                id: row

                readonly property int maxItemPreferredWidth: {
                    var mw = 0;
                    for (var i = 0; i < rowRepeater.count; i++) {
                        var item = rowRepeater.itemAt(i);
                        if (!item) {
                            continue;
                        }

                        mw = Math.max(mw, item.itemPreferredWidth);
                    }
                    return mw;
                }

                Layout.fillWidth: true
                Layout.fillHeight: false

                spacing: units.largeSpacing

                Repeater {
                    id: rowRepeater

                    model: rowData

                    delegate: Loader {
                        property var cellData: modelData
                        readonly property int itemPreferredWidth: (item.Layout && item.Layout.preferredWidth > 0) ? item.Layout.preferredWidth : item.implicitWidth
                        sourceComponent:
                            (rowIndex === 0) ? timeDelegate :
                            (rowIndex === 1) ? iconDelegate :
                            /* else */         temperatureDelegate

                        Layout.fillWidth: item.Layout.fillWidth
                        Layout.minimumWidth: maxItemPreferredWidth
                        Layout.minimumHeight: item.Layout.minimumHeight
                        Layout.alignment: item.Layout.alignment
                        Layout.preferredWidth: maxItemPreferredWidth
                        Layout.preferredHeight: item.Layout.preferredHeight
                    }
                }
            }
        }
    }
}
