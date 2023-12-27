/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root

    property alias model: categoryRepeater.model
    readonly property bool hasContent: model && model.length > 0 && (model[0].length > 0 || model[1].length > 0)

    spacing: Kirigami.Units.gridUnit

    Repeater {
        id: categoryRepeater

        delegate: ColumnLayout {
            property var categoryData: modelData

            readonly property bool categoryHasNotices: categoryData.length > 0
            visible: categoryHasNotices

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

            Kirigami.Heading {
                level: 4
                Layout.alignment: Qt.AlignHCenter

                text: index == 0 ? i18nc("@title:column weather warnings", "Warnings Issued") : i18nc("@title:column weather watches" ,"Watches Issued")
            }

            Repeater {
                id: repeater

                model: categoryData

                delegate: PlasmaComponents.Label {
                    font.underline: true
                    color: Kirigami.Theme.linkColor

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
