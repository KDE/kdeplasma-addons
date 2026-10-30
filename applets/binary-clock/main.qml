/*
 * SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
 * SPDX-FileCopyrightText: 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 *
 * Based on fuzzy-clock main.qml:
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
import org.kde.kirigami as Kirigami
import org.kde.plasma.clock
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.workspace.calendar as PlasmaCalendar

PlasmoidItem {
    id: root

    width: Kirigami.Units.gridUnit * 10
    height: Kirigami.Units.gridUnit * 4

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground
    preferredRepresentation: compactRepresentation

    // keep this consistent with toolTipMainText and toolTipSubText in analog-clock
    toolTipMainText: Qt.locale().toString(clockSource.dateTime, "dddd")
    toolTipSubText: Qt.locale().toString(clockSource.dateTime, Qt.locale().timeFormat(Locale.LongFormat)) + "\n" + Qt.locale().toString(clockSource.dateTime, Qt.locale().dateFormat(Locale.LongFormat).replace(/(^dddd.?\s)|(,?\sdddd$)/, ""))

    Clock {
        id: clockSource
        trackSeconds: Plasmoid.configuration.showSeconds || (root.compactRepresentationItem as BinaryClock).mouseArea.containsMouse
    }

    compactRepresentation: BinaryClock {
        property bool wasExpanded: false

        hours: clockSource.dateTime.getHours()
        minutes: clockSource.dateTime.getMinutes()
        seconds: clockSource.dateTime.getSeconds()
        mouseArea.onPressed: wasExpanded = root.expanded
        mouseArea.onClicked: root.expanded = !wasExpanded
    }

    fullRepresentation: PlasmaCalendar.MonthView {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 20
        Layout.minimumHeight: Kirigami.Units.gridUnit * 20

        today: clockSource.dateTime
    }
}
