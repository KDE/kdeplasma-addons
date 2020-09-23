/*
 *   SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Controls 2.5
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: configRoot
    anchors.left: parent.left
    anchors.right: parent.right

    property alias cfg_historySize: historySpin.value
    property alias cfg_copyAutomatically: copyAutomatically.checked

    SpinBox {
        id: historySpin
        Kirigami.FormData.label: i18nc("@label:spinbox", "History size:")
        value: 3
    }

    Item {
        Kirigami.FormData.isSection: false
    }

    CheckBox {
        id: copyAutomatically
        Kirigami.FormData.label: i18nc("@option:check", "Copy automatically:")
    }
}
