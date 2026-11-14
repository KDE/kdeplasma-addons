/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property alias cfg_temperatureUnit: temperatureComboBox.unit
    property alias cfg_pressureUnit: pressureComboBox.unit
    property alias cfg_speedUnit: windSpeedComboBox.unit
    property alias cfg_visibilityUnit: visibilityComboBox.unit

    property real comboboxMaxImplicitWidth: Math.max(temperatureComboBox.implicitWidth, pressureComboBox.implicitWidth, windSpeedComboBox.implicitWidth, visibilityComboBox.implicitWidth)

    Kirigami.Form {
        component UnitSelector: QQC2.ComboBox {
            property int unit

            Layout.preferredWidth: root.comboboxMaxImplicitWidth
            textRole: "display"
            currentIndex: model.listIndexForUnitId(unit)
            onActivated: unit = model.unitIdForListIndex(currentIndex)
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Temperature:")
                contentItem: UnitSelector {
                    id: temperatureComboBox
                    model: TemperatureUnitListModel
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Pressure:")
                contentItem: UnitSelector {
                    id: pressureComboBox
                    model: PressureUnitListModel
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Wind speed:")
                contentItem: UnitSelector {
                    id: windSpeedComboBox
                    model: WindSpeedUnitListModel
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Visibility:")
                contentItem: UnitSelector {
                    id: visibilityComboBox
                    model: VisibilityUnitListModel
                }
            }
        }
    }
}
