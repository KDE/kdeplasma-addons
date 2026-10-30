/*
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.clock
import org.kde.plasma.workspace.calendar as PlasmaCalendar

PlasmoidItem {
    id: root

    readonly property date currentDateTime: clockSource.dateTime

    width: Kirigami.Units.gridUnit * 10
    height: Kirigami.Units.gridUnit * 4

    preferredRepresentation: compactRepresentation

    // keep this consistent with toolTipMainText and toolTipSubText in analog-clock
    toolTipMainText: Qt.locale().toString(currentDateTime, "dddd")
    toolTipSubText: Qt.locale().toString(currentDateTime, Qt.locale().timeFormat(Locale.LongFormat)) + "\n" + Qt.locale().toString(currentDateTime, Qt.locale().dateFormat(Locale.LongFormat).replace(/(^dddd.?\s)|(,?\sdddd$)/, ""))

    Plasmoid.backgroundHints: PlasmaCore.Types.ShadowBackground | PlasmaCore.Types.ConfigurableBackground

    Clock {
        id: clockSource
        trackSeconds: (root.compactRepresentationItem as FuzzyClock).mouseArea.containsMouse
    }

    compactRepresentation: FuzzyClock {
        property bool wasExpanded: false

        dateTime: root.currentDateTime
        mouseArea.onPressed: wasExpanded = root.expanded
        mouseArea.onClicked: root.expanded = !wasExpanded
    }

    fullRepresentation: PlasmaCalendar.MonthView {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 20
        Layout.minimumHeight: Kirigami.Units.gridUnit * 20

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window

        today: root.currentDateTime
    }
}
