/*
 *   SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaExtras.MenuItem
{
    onClicked: {
        Qt.openUrlExternally(text);
    }
}
