/*
 * Copyright 2013 Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2013 Martin Klapetek <mklapetek@kde.org>
 * Copyright 2014 David Edmundson <davidedmundson@kde.org>
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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

Item {
    id: main

    Layout.minimumWidth: vertical ? 0 : sizehelper.paintedWidth + (units.smallSpacing * 2)
    Layout.maximumWidth: vertical ? Infinity : Layout.minimumWidth
    Layout.preferredWidth: vertical ? undefined : Layout.minimumWidth

    Layout.minimumHeight: vertical ? sizehelper.paintedHeight + (units.smallSpacing * 2) : 0
    Layout.maximumHeight: vertical ? Layout.minimumHeight : Infinity
    Layout.preferredHeight: vertical ? Layout.minimumHeight : theme.mSize(theme.defaultFont).height * 2

    property int formFactor: plasmoid.formFactor
    property int timePixelSize: theme.defaultFont.pixelSize

    property bool constrained: formFactor == PlasmaCore.Types.Vertical || formFactor == PlasmaCore.Types.Horizontal

    property bool vertical: plasmoid.formFactor == PlasmaCore.Types.Vertical

    property int fuzzyness: plasmoid.configuration.fuzzyness

    readonly property var hourNames: [
        i18nc("hour in the messages below","one"),
        i18nc("hour in the messages below","two"),
        i18nc("hour in the messages below","three"),
        i18nc("hour in the messages below","four"),
        i18nc("hour in the messages below","five"),
        i18nc("hour in the messages below","six"),
        i18nc("hour in the messages below","seven"),
        i18nc("hour in the messages below","eight"),
        i18nc("hour in the messages below","nine"),
        i18nc("hour in the messages below","ten"),
        i18nc("hour in the messages below","eleven"),
        i18nc("hour in the messages below","twelve")
    ]

    function normalFuzzy(fuzzy, hour) {
        switch(fuzzy) {
        case 0: return i18nc("%1 the hour translated above", "%1 o'clock", hour);
        case 1: return i18nc("%1 the hour translated above", "five past %1", hour);
        case 2: return i18nc("%1 the hour translated above", "ten past %1", hour);
        case 3: return i18nc("%1 the hour translated above", "quarter past %1", hour);
        case 4: return i18nc("%1 the hour translated above", "twenty past %1", hour);
        case 5: return i18nc("%1 the hour translated above", "twenty five past %1", hour);
        case 6: return i18nc("%1 the hour translated above", "half past %1", hour);
        case 7: return i18nc("%1 the hour translated above", "twenty five to %1", hour);
        case 8: return i18nc("%1 the hour translated above", "twenty to %1", hour);
        case 9: return i18nc("%1 the hour translated above", "quarter to %1", hour);
        case 10: return i18nc("%1 the hour translated above", "ten to %1", hour);
        case 11: return i18nc("%1 the hour translated above", "five to %1", hour);
        case 12: return i18nc("%1 the hour translated above", "%1 o'clock", hour);
        }
    }

    readonly property var dayTime: [
        i18n("Night"), i18n("Early morning"), i18n("Morning"), i18n("Almost noon"),
        i18n("Noon"), i18n("Afternoon"), i18n("Evening"), i18n("Late evening")
    ]

    readonly property var weekTime: [
        i18n("Start of week"), i18n("Middle of week"), i18n("End of week"), i18n("Weekend!")
    ]

    function timeString() {
        var upcaseFirst = i18nc("Whether to uppercase the first letter of " +
                                 "completed fuzzy time strings above: " +
                                 "translate as 1 if yes, 0 if no.",
                                 "1") != "0"

        var d = new Date(dataSource.data["Local"]["DateTime"])
        var hours = d.getHours()
        var minutes = d.getMinutes()

        if (main.fuzzyness == 1 || main.fuzzyness == 2) {
            var sector = 0
            var realHour = 0

            if (main.fuzzyness == 1) {
                if (minutes > 2) {
                    sector = (minutes - 3) / 5 + 1
                }
            } else {
                if (minutes > 6) {
                    sector = ((minutes - 7) / 15 + 1) * 3
                }
            }

            var deltaHour = (sector <= 6 ? 0 : 1)
            if ((hours + deltaHour) % 12 > 0) {
                realHour = (hours + deltaHour) % 12 - 1
            } else {
                realHour = 12 - ((hours + deltaHour) % 12 + 1)
            }

            var timeString = normalFuzzy(Math.floor(sector), hourNames[Math.floor(realHour)])
            if (upcaseFirst) {
                timeString = timeString[0].toUpperCase() + timeString.substr(1)
            }
            return timeString
        } else if (main.fuzzyness == 3) {
            return dayTime[Math.floor(hours / 3)]
        } else {
            var dow = d.getDay()

            var weekTimeId
            if (dow == 1) {
                weekTimeId = 0
            } else if (dow >= 2 && dow <= 4) {
                weekTimeId = 1
            } else if (dow == 5) {
                weekTimeId = 2
            } else {
                weekTimeId = 3
            }

            return weekTime[weekTimeId]
        }
    }

    Components.Label  {
        id: timeLabel
        font {
            weight: plasmoid.configuration.boldText ? Font.Bold : Font.Normal
            italic: plasmoid.configuration.italicText
            pixelSize: 1024
            pointSize: 0 // we need to unset pointSize otherwise it breaks the Text.Fit size mode
        }
        minimumPixelSize: theme.mSize(theme.smallestFont).height
        fontSizeMode: Text.Fit
        text: timeString()

        wrapMode: Text.NoWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        height: 0
        width: 0
        anchors {
            fill: parent
            leftMargin: units.smallSpacing
            rightMargin: units.smallSpacing
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: plasmoid.expanded = !plasmoid.expanded
    }

    Text {
        id: sizehelper
        font.weight: timeLabel.font.weight
        font.italic: timeLabel.font.italic
        font.pixelSize: vertical ? theme.mSize(theme.defaultFont).height * 2 : 1024 // random "big enough" size - this is used as a max pixelSize by the fontSizeMode
        minimumPixelSize: theme.mSize(theme.smallestFont).height
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: timeLabel.text
        fontSizeMode: vertical ? Text.HorizontalFit : Text.VerticalFit

        wrapMode: Text.NoWrap
        visible: false
        anchors {
            fill: parent
            leftMargin: units.smallSpacing
            rightMargin: units.smallSpacing
        }
    }
}
