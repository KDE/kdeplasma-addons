/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid
import plasma.applet.org.kde.plasma.comic as Comic

Item {
    id: root

    signal jumpClicked

    implicitWidth: 10
    implicitHeight: comicIdentifier.height

    property bool showUrl: false
    property bool showIdentifier: false
    property Comic.comicData comicData

    visible: (comicIdentifier.text.length > 0 || comicUrl.text.length > 0)

    PlasmaComponents3.Label {
        id: comicIdentifier

        anchors {
            left: root.left
            right: comicUrl.left
            leftMargin: 2
        }

        color: Kirigami.Theme.textColor
        visible: (root.showIdentifier && comicIdentifier.text.length > 0)
        text: (root.showIdentifier && root.comicData.currentReadable != undefined) ? root.comicData.currentReadable : ""
        textFormat: Text.PlainText
        elide: Text.ElideRight

        MouseArea {
            id: idLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = Kirigami.Theme.highlightColor;
            }

            onExited: {
                parent.color = Kirigami.Theme.textColor;
            }

            onClicked: {
                jumpClicked();
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: idLabelArea
                mainText: i18nc("@info:tooltip", "Jump to strip…")
            }
        }
    }

    PlasmaComponents3.Label {
        id:comicUrl

        anchors {
            right: root.right
            rightMargin: 2
        }

        color: Kirigami.Theme.textColor
        visible: (root.showUrl && comicUrl.text.length > 0)
        text: (root.showUrl && root.comicData.websiteHost?.length > 0) ? root.comicData.websiteHost : ""
        textFormat: Text.PlainText
        elide: Text.ElideRight

        MouseArea {
            id: idUrlLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = Kirigami.Theme.highlightColor;
            }

            onExited: {
                parent.color = Kirigami.Theme.textColor;
            }

            onClicked: {
                Plasmoid.shop();
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: idUrlLabelArea
                mainText: i18nc("@info:tooltip", "Visit the comic website")
            }
        }
    }
}
