/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    property alias model: warnings.model

    background: Rectangle {
        anchors.fill: parent
        color: "transparent"
    }

    ListView {
        id: warnings

        boundsBehavior: Flickable.StopAtBounds

        clip: false

        delegate: RowLayout {
            width: ListView.view.width
            spacing: 0

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignTop
                Layout.minimumWidth: Kirigami.Units.gridUnit * 5
                Layout.maximumWidth: Kirigami.Units.gridUnit * 10
                Layout.margins: Kirigami.Units.largeSpacing

                text: model.timestamp.toLocaleString(Qt.locale(), "ddd yyyy-MM-dd hh:mm:ss")
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignRight
                wrapMode: Text.Wrap
            }

            Kirigami.Icon {
                Layout.alignment: Qt.AlignTop
                Layout.minimumWidth: implicitWidth
                implicitWidth: Kirigami.Units.iconSizes.smallMedium
                source: (model.priority === Warnings.High) ? 'flag-red-symbolic' : (model.priority === Warnings.Medium) ? 'flag-yellow-symbolic' : 'flag-blue-symbolic'
            }

            Kirigami.SelectableLabel {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                Layout.minimumWidth: Kirigami.Units.gridUnit * 5
                Layout.margins: Kirigami.Units.largeSpacing

                text: model.description
                wrapMode: Text.Wrap
            }

            PlasmaComponents.ToolButton {
                visible: model.info !== ""
                Layout.alignment: Qt.AlignTop
                Layout.minimumWidth: implicitWidth
                icon.name: 'showinfo-symbolic'
                text: i18nc("@action:button", "Show more information")
                display: PlasmaComponents.ToolButton.IconOnly
                onClicked: {
                    Qt.openUrlExternally(Qt.resolvedUrl(model.info));
                }
            }
        }
    }
}
