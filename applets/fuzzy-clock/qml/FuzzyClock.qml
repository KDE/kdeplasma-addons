/*
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>
 * SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid

Item {
    id: main

    required property date dateTime

    Layout.minimumWidth: vertical ? 0 : sizehelper.paintedWidth + (Kirigami.Units.smallSpacing * 2)
    Layout.maximumWidth: vertical ? Infinity : Layout.minimumWidth
    Layout.preferredWidth: vertical ? undefined : Layout.minimumWidth

    Layout.minimumHeight: vertical ? sizehelper.paintedHeight + (Kirigami.Units.smallSpacing * 2) : 0
    Layout.maximumHeight: vertical ? Layout.minimumHeight : Infinity
    Layout.preferredHeight: vertical ? Layout.minimumHeight : Kirigami.Units.iconSizes.sizeForLabels * 2

    readonly property bool vertical: Plasmoid.formFactor == PlasmaCore.Types.Vertical

    readonly property int fuzzyness: Plasmoid.configuration.fuzzyness

    readonly property var hourNames: [
        [ i18nc("@info:status fuzzy clock time", "One o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past one"),
          i18nc("@info:status fuzzy clock time", "Ten past one"),
          i18nc("@info:status fuzzy clock time", "Quarter past one"),
          i18nc("@info:status fuzzy clock time", "Twenty past one"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past one"),
          i18nc("@info:status fuzzy clock time", "Half past one"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to two"),
          i18nc("@info:status fuzzy clock time", "Twenty to two"),
          i18nc("@info:status fuzzy clock time", "Quarter to two"),
          i18nc("@info:status fuzzy clock time", "Ten to two"),
          i18nc("@info:status fuzzy clock time", "Five to two") ],
        [ i18nc("@info:status fuzzy clock time", "Two o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past two"),
          i18nc("@info:status fuzzy clock time", "Ten past two"),
          i18nc("@info:status fuzzy clock time", "Quarter past two"),
          i18nc("@info:status fuzzy clock time", "Twenty past two"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past two"),
          i18nc("@info:status fuzzy clock time", "Half past two"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to three"),
          i18nc("@info:status fuzzy clock time", "Twenty to three"),
          i18nc("@info:status fuzzy clock time", "Quarter to three"),
          i18nc("@info:status fuzzy clock time", "Ten to three"),
          i18nc("@info:status fuzzy clock time", "Five to three") ],
        [ i18nc("@info:status fuzzy clock time", "Three o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past three"),
          i18nc("@info:status fuzzy clock time", "Ten past three"),
          i18nc("@info:status fuzzy clock time", "Quarter past three"),
          i18nc("@info:status fuzzy clock time", "Twenty past three"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past three"),
          i18nc("@info:status fuzzy clock time", "Half past three"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to four"),
          i18nc("@info:status fuzzy clock time", "Twenty to four"),
          i18nc("@info:status fuzzy clock time", "Quarter to four"),
          i18nc("@info:status fuzzy clock time", "Ten to four"),
          i18nc("@info:status fuzzy clock time", "Five to four") ],
        [ i18nc("@info:status fuzzy clock time", "Four o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past four"),
          i18nc("@info:status fuzzy clock time", "Ten past four"),
          i18nc("@info:status fuzzy clock time", "Quarter past four"),
          i18nc("@info:status fuzzy clock time", "Twenty past four"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past four"),
          i18nc("@info:status fuzzy clock time", "Half past four"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to five"),
          i18nc("@info:status fuzzy clock time", "Twenty to five"),
          i18nc("@info:status fuzzy clock time", "Quarter to five"),
          i18nc("@info:status fuzzy clock time", "Ten to five"),
          i18nc("@info:status fuzzy clock time", "Five to five") ],
        [ i18nc("@info:status fuzzy clock time", "Five o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past five"),
          i18nc("@info:status fuzzy clock time", "Ten past five"),
          i18nc("@info:status fuzzy clock time", "Quarter past five"),
          i18nc("@info:status fuzzy clock time", "Twenty past five"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past five"),
          i18nc("@info:status fuzzy clock time", "Half past five"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to six"),
          i18nc("@info:status fuzzy clock time", "Twenty to six"),
          i18nc("@info:status fuzzy clock time", "Quarter to six"),
          i18nc("@info:status fuzzy clock time", "Ten to six"),
          i18nc("@info:status fuzzy clock time", "Five to six") ],
        [ i18nc("@info:status fuzzy clock time", "Six o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past six"),
          i18nc("@info:status fuzzy clock time", "Ten past six"),
          i18nc("@info:status fuzzy clock time", "Quarter past six"),
          i18nc("@info:status fuzzy clock time", "Twenty past six"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past six"),
          i18nc("@info:status fuzzy clock time", "Half past six"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to seven"),
          i18nc("@info:status fuzzy clock time", "Twenty to seven"),
          i18nc("@info:status fuzzy clock time", "Quarter to seven"),
          i18nc("@info:status fuzzy clock time", "Ten to seven"),
          i18nc("@info:status fuzzy clock time", "Five to seven") ],
        [ i18nc("@info:status fuzzy clock time", "Seven o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past seven"),
          i18nc("@info:status fuzzy clock time", "Ten past seven"),
          i18nc("@info:status fuzzy clock time", "Quarter past seven"),
          i18nc("@info:status fuzzy clock time", "Twenty past seven"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past seven"),
          i18nc("@info:status fuzzy clock time", "Half past seven"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to eight"),
          i18nc("@info:status fuzzy clock time", "Twenty to eight"),
          i18nc("@info:status fuzzy clock time", "Quarter to eight"),
          i18nc("@info:status fuzzy clock time", "Ten to eight"),
          i18nc("@info:status fuzzy clock time", "Five to eight") ],
        [ i18nc("@info:status fuzzy clock time", "Eight o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past eight"),
          i18nc("@info:status fuzzy clock time", "Ten past eight"),
          i18nc("@info:status fuzzy clock time", "Quarter past eight"),
          i18nc("@info:status fuzzy clock time", "Twenty past eight"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past eight"),
          i18nc("@info:status fuzzy clock time", "Half past eight"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to nine"),
          i18nc("@info:status fuzzy clock time", "Twenty to nine"),
          i18nc("@info:status fuzzy clock time", "Quarter to nine"),
          i18nc("@info:status fuzzy clock time", "Ten to nine"),
          i18nc("@info:status fuzzy clock time", "Five to nine") ],
        [ i18nc("@info:status fuzzy clock time", "Nine o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past nine"),
          i18nc("@info:status fuzzy clock time", "Ten past nine"),
          i18nc("@info:status fuzzy clock time", "Quarter past nine"),
          i18nc("@info:status fuzzy clock time", "Twenty past nine"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past nine"),
          i18nc("@info:status fuzzy clock time", "Half past nine"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to ten"),
          i18nc("@info:status fuzzy clock time", "Twenty to ten"),
          i18nc("@info:status fuzzy clock time", "Quarter to ten"),
          i18nc("@info:status fuzzy clock time", "Ten to ten"),
          i18nc("@info:status fuzzy clock time", "Five to ten") ],
        [ i18nc("@info:status fuzzy clock time", "Ten o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past ten"),
          i18nc("@info:status fuzzy clock time", "Ten past ten"),
          i18nc("@info:status fuzzy clock time", "Quarter past ten"),
          i18nc("@info:status fuzzy clock time", "Twenty past ten"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past ten"),
          i18nc("@info:status fuzzy clock time", "Half past ten"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to eleven"),
          i18nc("@info:status fuzzy clock time", "Twenty to eleven"),
          i18nc("@info:status fuzzy clock time", "Quarter to eleven"),
          i18nc("@info:status fuzzy clock time", "Ten to eleven"),
          i18nc("@info:status fuzzy clock time", "Five to eleven") ],
        [ i18nc("@info:status fuzzy clock time", "Eleven o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past eleven"),
          i18nc("@info:status fuzzy clock time", "Ten past eleven"),
          i18nc("@info:status fuzzy clock time", "Quarter past eleven"),
          i18nc("@info:status fuzzy clock time", "Twenty past eleven"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past eleven"),
          i18nc("@info:status fuzzy clock time", "Half past eleven"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to twelve"),
          i18nc("@info:status fuzzy clock time", "Twenty to twelve"),
          i18nc("@info:status fuzzy clock time", "Quarter to twelve"),
          i18nc("@info:status fuzzy clock time", "Ten to twelve"),
          i18nc("@info:status fuzzy clock time", "Five to twelve") ],
        [ i18nc("@info:status fuzzy clock time", "Twelve o’clock"),
          i18nc("@info:status fuzzy clock time", "Five past twelve"),
          i18nc("@info:status fuzzy clock time", "Ten past twelve"),
          i18nc("@info:status fuzzy clock time", "Quarter past twelve"),
          i18nc("@info:status fuzzy clock time", "Twenty past twelve"),
          i18nc("@info:status fuzzy clock time", "Twenty-five past twelve"),
          i18nc("@info:status fuzzy clock time", "Half past twelve"),
          i18nc("@info:status fuzzy clock time", "Twenty-five to one"),
          i18nc("@info:status fuzzy clock time", "Twenty to one"),
          i18nc("@info:status fuzzy clock time", "Quarter to one"),
          i18nc("@info:status fuzzy clock time", "Ten to one"),
          i18nc("@info:status fuzzy clock time", "Five to one") ]
    ]

    readonly property var halflingTime: [
        i18nc("@info:status fuzzy clock time", "Sleep"), i18nc("@info:status fuzzy clock time", "Breakfast"), i18nc("@info:status fuzzy clock time", "Second Breakfast"), i18nc("@info:status fuzzy clock time", "Elevenses"),
        i18nc("@info:status fuzzy clock time", "Lunch"), i18nc("@info:status fuzzy clock time", "Afternoon tea"), i18nc("@info:status fuzzy clock time", "Dinner"), i18nc("@info:status fuzzy clock time", "Supper")
    ]

    readonly property var dayTime: [
        i18nc("@info:status fuzzy clock time", "Night"), i18nc("@info:status fuzzy clock time", "Early morning"), i18nc("@info:status fuzzy clock time", "Morning"), i18nc("@info:status fuzzy clock time", "Almost noon"),
        i18nc("@info:status fuzzy clock time", "Noon"), i18nc("@info:status fuzzy clock time", "Afternoon"), i18nc("@info:status fuzzy clock time", "Evening"), i18nc("@info:status fuzzy clock time", "Late evening")
    ]

    readonly property var weekTime: [
        i18nc("@info:status fuzzy clock time", "Start of week"), i18nc("@info:status fuzzy clock time", "Middle of week"), i18nc("@info:status fuzzy clock time", "End of week"), i18nc("@info:status fuzzy clock time", "Weekend!")
    ]

    function timeString() {
        var d = new Date(main.dateTime)
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
                // this formula has been determined by carefully filling a spreadsheet
                // and looking at the numbers :)
                sector = ((minutes + 7) / 15 * 3)
                // now round down to the nearest three
                sector = (Math.floor(sector / 3) * 3)
            }

            if (hours % 12 > 0) {
                realHour = hours % 12 - 1
            } else {
                realHour = 12 - (hours % 12 + 1)
            }

            sector = Math.floor(sector);
            if (sector == 12) {
                realHour += 1
                if (Math.floor(realHour) >= hourNames.length) {
                    realHour = 0
                }
                sector = 0
            }

            return hourNames[Math.floor(realHour)][sector]
        } else if (main.fuzzyness == 3) {
            return halflingTime[Math.floor(hours / 3)]
        } else if (main.fuzzyness == 4) {
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

    activeFocusOnTab: true

    Accessible.name: Plasmoid.title
    Accessible.description: timeLabel.text
    Accessible.role: Accessible.Button

    PlasmaComponents3.Label  {
        id: timeLabel
        font {
            weight: Plasmoid.configuration.boldText ? Font.Bold : Font.Normal
            italic: Plasmoid.configuration.italicText
            pixelSize: 1024
        }
        minimumPixelSize: Kirigami.Units.iconSizes.sizeForLabels
        fontSizeMode: Text.Fit
        text: main.timeString()

        wrapMode: Text.NoWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        height: 0
        width: 0
        anchors {
            fill: parent
            leftMargin: Kirigami.Units.smallSpacing
            rightMargin: Kirigami.Units.smallSpacing
        }
    }

    readonly property alias mouseArea: mouseArea

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    Text {
        id: sizehelper
        font.weight: timeLabel.font.weight
        font.italic: timeLabel.font.italic
        font.pixelSize: main.vertical ? Kirigami.Units.gridUnit * 2 : 1024 // random "big enough" size - this is used as a max pixelSize by the fontSizeMode
        minimumPixelSize: Math.round(Kirigami.Units.gridUnit / 2)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: timeLabel.text
        fontSizeMode: main.vertical ? Text.HorizontalFit : Text.VerticalFit

        wrapMode: Text.NoWrap
        visible: false
        anchors {
            fill: parent
            leftMargin: Kirigami.Units.smallSpacing
            rightMargin: Kirigami.Units.smallSpacing
        }
    }
}
