/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kcmutils as KCM
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcmutils as KCM

KCM.GridViewKCM {
    id: kcm
    property string cfg_color
    property alias cfg_fontSize: fontSizeSpinBox.value

    readonly property bool inPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.RightEdge,PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge].includes(Plasmoid.location)

    extraFooterTopPadding: true

    header: Kirigami.FormLayout {
        QQC2.SpinBox {
            id: fontSizeSpinBox

            implicitWidth: Kirigami.Units.gridUnit * 3
            from: 4
            to: 128
            textFromValue: function (value) {
                return i18n("%1pt", value)
            }
            valueFromText: function (text) {
                return parseInt(text)
            }

            Kirigami.FormData.label: i18n("Text font size:")
        }
    }

    view.implicitCellWidth: Kirigami.Units.gridUnit * 8
    view.implicitCellHeight: view.implicitCellWidth

    view.model: {
        let model = ["white", "black", "red", "orange", "yellow", "green", "blue", "pink", "translucent"];
        if (!kcm.inPanel) {
            model.push("translucent-light");
        }
        return model;
    }
    view.currentIndex: {
        let color = cfg_color;
        if (kcm.inPanel && color === "translucent-light") {
            color = "translucent";
        }
        return view.model.indexOf(color);
    }
    view.onCurrentIndexChanged: cfg_color = view.model[view.currentIndex]

    view.delegate: QQC2.ItemDelegate {
        id: delegate
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight
        highlighted: GridView.isCurrentItem

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
            case "translucent": return i18n("A transparent sticky note")
            case "translucent-light": return i18n("A transparent sticky note with light text")
            }
        }

        contentItem: null

        KSvg.SvgItem {
            id: thumbnail

            anchors.centerIn: parent
            width: height
            height: parent.height - Kirigami.Units.largeSpacing
            imagePath: "widgets/notes"
            elementId: modelData + "-notes"

            QQC2.Label {
                id: thumbnailLabel
                anchors.centerIn: parent
                // this isn't a frameSVG, the default SVG margins take up around 7% of the frame size, so we use that
                width: Math.round(parent.width - thumbnail.width * 0.07) - Kirigami.Units.smallSpacing
                height: Math.round(parent.height - thumbnail.height * 0.07)
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: delegate.text
                textFormat: Text.PlainText
                elide: Text.ElideRight
                wrapMode: Text.WordWrap

                //this is deliberately _NOT_ the theme color as we are over a known bright background
                //an unknown colour over a known colour is a bad move as you end up with white on yellow
                color: {
                    if (inPanel && modelData === "translucent") {
                        return Kirigami.Theme.textColor;
                    } else if (modelData === "black" || modelData === "translucent-light") {
                        return "#dfdfdf"
                    } else {
                        return "#202020"
                    }
                }
            }
        }
        onClicked: {
            cfg_color = modelData
        }
    }
}
