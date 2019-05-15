/*
 * Copyright 2013  Bhushan Shah <bhush94@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: appearancePage

    property alias cfg_boldText: boldCheckBox.checked
    property alias cfg_italicText: italicCheckBox.checked

    property alias cfg_fuzzyness: fuzzyness.value

    QQC2.CheckBox {
        id: boldCheckBox
        Kirigami.FormData.label: i18nc("@title:group", "Font:")
        text: i18nc("@option:check", "Bold text")
    }

    QQC2.CheckBox {
        id: italicCheckBox
        text: i18nc("@option:check", "Italic text")
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.Slider {
        id: fuzzyness
        Kirigami.FormData.label: i18nc("@title:group", "Fuzzyness:")
        from: 1
        to: 5
        stepSize: 1
    }

    RowLayout {
       Layout.fillWidth: true

        QQC2.Label {
            text: i18nc("@item:inrange", "Accurate")
        }

        Item {
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: i18nc("@item:inrange", "Fuzzy")
        }
    }
}
