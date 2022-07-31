/*
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
// nothing used, but done to trigger imageprovider addition in plugin init
import org.kde.plasma.private.fifteenpuzzle 0.1 as Private

Item {
    id: root

    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 16 - 20
    Plasmoid.switchHeight: Plasmoid.switchWidth

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Plasmoid.toolTipMainText: i18n("Fifteen Puzzle");
    Plasmoid.toolTipSubText: i18n("Solve by arranging in order");

    Plasmoid.fullRepresentation: FifteenPuzzle { }
}
