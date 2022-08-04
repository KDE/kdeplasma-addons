/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15

import QtQuick.Layouts 1.3

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Loader {
    id: root

    property alias model: categoryListView.model
    readonly property bool hasContent: model && model.length > 0 && (model[0].length > 0 || model[1].length > 0)

    active: activeFocus
    activeFocusOnTab: isCurrentItem
    asynchronous: true

    sourceComponent: PlasmaExtras.Highlight {
        hovered: true
    }

    Accessible.description: {
        let description = [];
        if (model[0].length > 0) {
            model[0].forEach((data) => {
                description.push(i18nc("@title:column weather warnings", "Warnings Issued"));
                description.push(data.description);
            });
        }
        if (model[1].length > 0) {
            model[1].forEach((data) => {
                description.push(i18nc("@title:column weather warnings", "Watches Issued"));
                description.push(data.description);
            });
        }
        return description.join(" ");
    }

    PlasmaComponents.ScrollView {
        anchors.fill: parent

        ListView {
            id: categoryListView
            width: parent.width

            delegate: ColumnLayout {
                property var categoryData: modelData

                readonly property bool categoryHasNotices: categoryData.length > 0
                visible: categoryHasNotices

                width: parent.width
                height: implicitHeight

                PlasmaExtras.Heading {
                    Layout.fillWidth: true
                    level: 4

                    horizontalAlignment: Text.AlignHCenter
                    text: index == 0 ? i18nc("@title:column weather warnings", "Warnings Issued") : i18nc("@title:column weather watches" ,"Watches Issued")
                }

                Repeater {
                    id: repeater

                    model: categoryData

                    delegate: PlasmaComponents.Label {
                        Layout.fillWidth: true

                        font.underline: true
                        color: PlasmaCore.Theme.linkColor

                        text: modelData.description
                        wrapMode: Text.Wrap

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
    }
}
