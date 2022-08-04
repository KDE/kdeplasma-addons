/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {
    id: root

    property alias model: repeater.model
    readonly property int preferredIconSize: PlasmaCore.Units.iconSizes.large
    readonly property bool hasContent: model && model.length > 0

    spacing: PlasmaCore.Units.smallSpacing

    Accessible.description: {
        if (model.length === 0) {
            return "";
        }

        let description = [];
        for (let i = 0; i < model[0].length; i++) {
            const dayNumber = parseInt(model[0][i]);
            if (isNaN(dayNumber)) {
                // Day/Night label
                description.push(model[0][i]);
            } else {
                const today = new Date();
                const date = new Date(today.getFullYear(), today.getMonth(), dayNumber);
                description.push(Qt.locale().standaloneDayName(date.getDay()));
            }
            description.push(": ");

            if (model[1].length > 0) {
                description.push(i18nc("@info", "Weather condition: %1; ", model[1][i].text));
            }
            if (model[2].length > 0) {
                description.push(i18nc("@info", "Highest temperature: %1; ", model[2][i]));
            }
            if (model[3].length > 0) {
                description.push(i18nc("@info", "Lowest temperature: %1; ", model[3][i]));
            }
        }
        return description.join("");
    }

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

            source: cellData.icon

            PlasmaCore.ToolTipArea {
                id: iconToolTip

                anchors.fill: parent

                mainText: cellData.text
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

                spacing: PlasmaCore.Units.largeSpacing

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
