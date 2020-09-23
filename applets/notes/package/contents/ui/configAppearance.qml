/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kcm 1.1 as KCM
import org.kde.plasma.core 2.0 as PlasmaCore

KCM.GridView {
    id: appearancePage

    property string cfg_color

    contentWidth: implicitWidth
    contentHeight: implicitHeight

    PlasmaCore.Svg {
        id: noteSvg
        imagePath: "widgets/notes"
    }

    view.model: ["white", "black", "red", "orange", "yellow", "green", "blue", "pink", "translucent", "translucent-light"]
    view.currentIndex: view.model.indexOf(cfg_color)
    view.onCurrentIndexChanged: cfg_color = view.model[view.currentIndex]

    view.delegate: KCM.GridDelegate {
        id: delegate
        thumbnailAvailable: true
        thumbnail: PlasmaCore.SvgItem {
            anchors.fill: parent
            anchors.margins: units.gridUnit / 2
            
            svg: noteSvg
            elementId: modelData + "-notes"

            QQC2.Label {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

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
                elide: Text.ElideRight
                wrapMode: Text.WordWrap

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
        onClicked: {
            cfg_color = modelData
            appearancePage.forceActiveFocus();
        }
    }
}
