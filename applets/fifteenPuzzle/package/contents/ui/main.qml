/*
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
// nothing used, but done to trigger imageprovider addition in plugin init
import org.kde.plasma.private.fifteenpuzzle 0.1 as Private

Item {
    id: root

    Plasmoid.switchWidth: Layout.minimumWidth - 20
    Plasmoid.switchHeight: Layout.minimumHeight - 20

    Layout.minimumWidth: width
    Layout.minimumHeight: height

    width: PlasmaCore.Units.gridUnit * 16
    height: PlasmaCore.Units.gridUnit * 16

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Plasmoid.toolTipMainText: i18n("Fifteen Puzzle");
    Plasmoid.toolTipSubText: i18n("Solve by arranging in order");

    PlasmaCore.DataSource {
        id: dataSource
    }

    Plasmoid.fullRepresentation: FifteenPuzzle { }
}
