/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QtControls

import org.kde.kirigami 2.15 as Kirigami
import org.kde.plasma.private.weather 1.0


Kirigami.FormLayout {
    id: unitsConfigPage

    property alias cfg_temperatureUnit: temperatureComboBox.unit
    property alias cfg_pressureUnit: pressureComboBox.unit
    property alias cfg_speedUnit: windSpeedComboBox.unit
    property alias cfg_visibilityUnit: visibilityComboBox.unit

    component UnitSelector: QtControls.ComboBox {
        property int unit

        textRole: "display"
        currentIndex: model.listIndexForUnitId(unit)
        onActivated: unit = model.unitIdForListIndex(currentIndex)
    }

    UnitSelector {
        id: temperatureComboBox
        Kirigami.FormData.label: i18nc("@label:listbox", "Temperature:")
        model: TemperatureUnitListModel
    }

    UnitSelector {
        id: pressureComboBox
        Kirigami.FormData.label: i18nc("@label:listbox", "Pressure:")
        model: PressureUnitListModel
    }

    UnitSelector {
        id: windSpeedComboBox
        Kirigami.FormData.label: i18nc("@label:listbox", "Wind speed:")
        model: WindSpeedUnitListModel
    }

    UnitSelector {
        id: visibilityComboBox
        Kirigami.FormData.label: i18nc("@label:listbox", "Visibility:")
        model: VisibilityUnitListModel
    }
}
