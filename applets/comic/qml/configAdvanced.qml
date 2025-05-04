/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts as Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property alias cfg_maxComicLimit: maxComicLimit.value

    signal configurationChanged

    Kirigami.FormLayout {
        Layouts.RowLayout {
            Kirigami.FormData.label: i18nc("@label:spinbox", "Comic cache:")

            Controls.SpinBox {
                id: maxComicLimit
                stepSize: 1
                onValueChanged: root.configurationChanged();
            }

            Controls.Label {
                text: i18ncp("@item:valuesuffix spacing to number + unit", "strip per comic", "strips per comic")
                textFormat: Text.PlainText
            }
        }
    }
}
