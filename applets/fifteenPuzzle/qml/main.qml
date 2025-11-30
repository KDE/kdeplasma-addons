/*
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
// nothing used, but done to trigger imageprovider addition in plugin init
import org.kde.plasma.private.fifteenpuzzle as Private

PlasmoidItem {
    id: root

    width: Kirigami.Units.gridUnit * 16 - 20
    height: Kirigami.Units.gridUnit * 16 - 20

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    readonly property bool compactInPanel: inPanel && !!compactRepresentationItem?.visible

    Plasmoid.icon: inPanel ? "fifteenpuzzle-symbolic" : "fifteenpuzzle"
    
    preferredRepresentation: fullRepresentation

    toolTipMainText: i18n("Fifteen Puzzle");
    toolTipSubText: i18n("Solve by arranging in order");

    fullRepresentation: FifteenPuzzle { }
}
