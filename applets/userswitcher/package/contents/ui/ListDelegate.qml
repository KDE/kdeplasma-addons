/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

PlasmaComponents3.ItemDelegate {
    id: item

    Layout.fillWidth: true

    property alias subText: sublabel.text
    property alias iconItem: iconItem.children

    highlighted: activeFocus

    Accessible.name: `${text}${subText ? `: ${subText}` : ""}`

    onHoveredChanged: if (hovered) {
        if (ListView.view) {
            ListView.view.currentIndex = index;
        }
        forceActiveFocus();
    }

    contentItem: RowLayout {
        id: row

        spacing: PlasmaCore.Units.smallSpacing

        Item {
            id: iconItem

            Layout.preferredWidth: PlasmaCore.Units.iconSizes.medium
            Layout.preferredHeight: PlasmaCore.Units.iconSizes.medium
            Layout.minimumWidth: Layout.preferredWidth
            Layout.maximumWidth: Layout.preferredWidth
            Layout.minimumHeight: Layout.preferredHeight
            Layout.maximumHeight: Layout.preferredHeight
        }

        ColumnLayout {
            id: column
            Layout.fillWidth: true
            spacing: 0

            PlasmaComponents3.Label {
                id: label
                Layout.fillWidth: true
                text: item.text
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
            }

            PlasmaComponents3.Label {
                id: sublabel
                Layout.fillWidth: true
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
                opacity: 0.6
                font: PlasmaCore.Theme.smallestFont
                visible: text !== ""
            }
        }
    }
}
