/*
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQml
import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.workspace.calendar 2.0 as PlasmaCalendar

Item {
    id: root

    readonly property date currentDateTime: dataSource.data.Local ? dataSource.data.Local.DateTime : new Date()

    width: PlasmaCore.Units.gridUnit * 10
    height: PlasmaCore.Units.gridUnit * 4

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    // keep this consistent with toolTipMainText and toolTipSubText in analog-clock
    Plasmoid.toolTipMainText: Qt.formatDate(currentDateTime, "dddd")
    Plasmoid.toolTipSubText: Qt.formatTime(currentDateTime,  Qt.locale().timeFormat(Locale.LongFormat)) + "\n" + Qt.formatDate(currentDateTime, Qt.locale().dateFormat(Locale.LongFormat).replace(/(^dddd.?\s)|(,?\sdddd$)/, ""))

    Plasmoid.backgroundHints: PlasmaCore.Types.ShadowBackground | PlasmaCore.Types.ConfigurableBackground

    P5Support.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: Plasmoid.compactRepresentationItem.mouseArea.containsMouse ? 1000 : 60000
        intervalAlignment: Plasmoid.compactRepresentationItem.mouseArea.containsMouse ? PlasmaCore.Types.NoAlignment : PlasmaCore.Types.AlignToMinute
    }

    Plasmoid.compactRepresentation: FuzzyClock { }

    Plasmoid.fullRepresentation: PlasmaCalendar.MonthView {
        Layout.minimumWidth: PlasmaCore.Units.gridUnit * 20
        Layout.minimumHeight: PlasmaCore.Units.gridUnit * 20

        PlasmaCore.ColorScope.inherit: false
        PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.NormalColorGroup

        today: currentDateTime
    }
}
