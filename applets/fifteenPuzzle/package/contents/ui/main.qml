/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

    width: units.gridUnit * 16
    height: units.gridUnit * 16

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Plasmoid.toolTipMainText: i18n("Fifteen Puzzle");
    Plasmoid.toolTipSubText: i18n("Solve by arranging in order");

    PlasmaCore.DataSource {
        id: dataSource
    }

    Plasmoid.fullRepresentation: FifteenPuzzle { }
}
