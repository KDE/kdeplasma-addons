/*
 * Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Controls 1.3 as QtControls
import QtQuick.Layouts 1.1 as QtLayouts

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

QtLayouts.ColumnLayout {
    id: appearancePage

    property string cfg_color

    PlasmaCore.Svg {
        id: noteSvg
        imagePath: "widgets/notes"
    }

    SystemPalette {
        id: syspal
    }

    QtControls.ScrollView {
        id: notesView
        QtLayouts.Layout.fillWidth: true
        QtLayouts.Layout.fillHeight: true
        frameVisible: true
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn // otherwise we get stuck in a re-layout loop
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        GridView {
            id: notesGrid
            width: notesView.width
            height: notesView.height
            cacheBuffer: 2000 // sometimes GridView gets confused and layouts items *somewhere*
            cellWidth: Math.floor(notesView.viewport.width / 3)
            cellHeight: cellWidth
            model: ["white", "black", "red", "orange", "yellow", "green", "blue", "pink", "translucent", "translucent-light"]
            currentIndex: model.indexOf(cfg_color)
            onCurrentIndexChanged: cfg_color = model[currentIndex]

            highlight: Rectangle {
                color: syspal.highlight
            }
            highlightMoveDuration: 0

            delegate: MouseArea {
                width: notesGrid.cellWidth
                height: notesGrid.cellHeight
                hoverEnabled: true
                onClicked: notesGrid.currentIndex = index

                Rectangle {
                    anchors.fill: parent
                    color: syspal.highlight
                    opacity: 0.5
                    visible: parent.containsMouse
                }

                PlasmaCore.SvgItem {
                    anchors {
                        fill: parent
                        margins: units.gridUnit / 2
                    }
                    svg: noteSvg
                    elementId: modelData + "-notes"

                    PlasmaComponents.Label {
                        anchors {
                            fill: parent
                            //this isn't a frameSVG, the default SVG margins take up around 7% of the frame size, so we use that
                            margins: parent.width * 0.07
                        }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        text: {
                            switch (modelData) {
                            case "white": return i18n("A white sticky note")
                            case "black": return i18n("A black sticky note")
                            case "red": return i18n("A red sticky note")
                            case "orange": return i18n("An orange sticky note")
                            case "yellow": return i18n("A yellow sticky note")
                            case "green": return i18n("A green sticky note")
                            case "blue": return i18n("A blue sticky note")
                            case "pink": return i18n("A pink sticky note")
                            case "translucent": return i18n("A translucent sticky note")
                            case "translucent-light": return i18n("A translucent sticky note with light text")
                            }
                        }
                        //this is deliberately _NOT_ the theme color as we are over a known bright background
                        //an unknown colour over a known colour is a bad move as you end up with white on yellow
                        color: {
                            if (modelData === "black" || modelData === "translucent-light") {
                                return "#dfdfdf"
                            } else {
                                return "#202020"
                            }
                        }
                    }
                }
            }
        }
    }
}
