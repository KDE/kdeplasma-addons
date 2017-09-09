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
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Layouts 1.0 as QtLayouts

Item {
    id: appearancePage
    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_boldText: boldCheckBox.checked
    property alias cfg_italicText: italicCheckBox.checked

    property alias cfg_fuzzyness: fuzzyness.value

    QtLayouts.ColumnLayout {
        QtControls.GroupBox {
            title: i18n("Appearance")
            flat: true

            QtLayouts.ColumnLayout {
                QtControls.CheckBox {
                    id: boldCheckBox
                    text: i18n("Bold text")
                }

                QtControls.CheckBox {
                    id: italicCheckBox
                    text: i18n("Italic text")
                }
            }
        }

        QtControls.GroupBox {
            title: i18n("Fuzzyness")
            flat: true

            QtLayouts.RowLayout {
                QtControls.Label {
                    text: i18n("Accurate")
                }

                QtControls.Slider {
                    id: fuzzyness
                    QtLayouts.Layout.fillWidth: true
                    minimumValue: 1
                    maximumValue: 5
                    stepSize: 1
                    tickmarksEnabled: true
                }

                QtControls.Label {
                    text: i18n("Fuzzy")
                }
            }
        }
    }
}
