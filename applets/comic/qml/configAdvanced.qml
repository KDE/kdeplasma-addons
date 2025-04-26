/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid
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
