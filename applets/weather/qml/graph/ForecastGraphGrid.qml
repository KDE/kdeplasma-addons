/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.kirigami as Kirigami

Item {
    id: root

    required property int horizontalLinesNumber
    required property int verticalLinesNumber

    Repeater {
        model: root.verticalLinesNumber

        delegate: Rectangle {
            required property int index

            color: Kirigami.Theme.activeBackgroundColor

            // Draw a 1-pixel vertical grid line
            width: 1
            height: root.height

            x: {
                const ratio = index / (root.verticalLinesNumber - 1);
                return ratio * root.width - width / 2;
            }
        }
    }

    Repeater {
        model: root.horizontalLinesNumber

        delegate: Rectangle {
            required property int index

            color: Kirigami.Theme.activeBackgroundColor

            // Draw a 1-pixel horizontal grid line
            height: 1
            width: root.width

            y: {
                const ratio = index / (root.horizontalLinesNumber - 1);
                return ratio * root.height - height / 2;
            }
        }
    }
}
