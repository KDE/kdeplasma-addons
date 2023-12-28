/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {
    id: root

    required property var model

    GridLayout {
        Layout.alignment: Qt.AlignCenter

        rows: labelRepeater.count
        flow: GridLayout.TopToBottom
        rowSpacing: Kirigami.Units.smallSpacing

        Repeater {
            id: labelRepeater
            model: root.model
            delegate: PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                text: modelData.label
            }
        }

        Repeater {
            model: root.model
            delegate: PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                text: modelData.text
            }
        }
    }
}
