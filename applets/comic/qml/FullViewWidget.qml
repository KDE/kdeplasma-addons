/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents
import org.kde.kquickcontrolsaddons
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami

PlasmaCore.Dialog {
    id: root

    property alias image: comicPicture.image
    property bool isError
    property string identifier
    property int identifierType

    flags: Qt.Popup
    visible: false
    hideOnWindowDeactivate: true

    function toggleVisibility()
    {
        root.visible = !root.visible;
        if (root.visible) {
            Plasmoid.positionFullView(root);
        }
    }

    function close() {
        root.visible = false;
    }

    mainItem: PlasmaComponents.ScrollView {
        id: mainScrollArea

        anchors.fill: parent

        // 4×gridUnit in case image is empty, to show something and avoid anchor loops.
        // value is arbitrary, but small enough to be unlikely to increase popup size for real comics
        Layout.minimumWidth: Math.max(Kirigami.Units.gridUnit * 4, viewContainer.contentWidth)
        Layout.maximumWidth: Layout.minimumWidth
        Layout.minimumHeight: Math.max(Kirigami.Units.gridUnit * 4, viewContainer.contentHeight)
        Layout.maximumHeight: Layout.minimumHeight

        Flickable {
            id: viewContainer

            anchors.fill: parent

            contentWidth: root.isError ? errorPlaceholder.implicitWidth + 2 * Kirigami.Units.largeSpacing : comicPicture.nativeWidth
            contentHeight: root.isError ? errorPlaceholder.implicitHeight  + 2 * Kirigami.Units.largeSpacing : comicPicture.nativeHeight

            //clip: true

            QImageItem {
                id: comicPicture

                anchors.fill: parent
                visible: !root.isError

                smooth: true
                fillMode: QImageItem.PreserveAspectFit

                MouseArea {
                    id: dialogMouseArea

                    anchors.fill: comicPicture

                    onClicked: {
                        root.close();
                    }
                }
            }

            ErrorPlaceholder {
                id: errorPlaceholder
                anchors.centerIn: parent
                visible: root.isError
                identifier: root.identifier
                identifierType: root.identifierType
            }
        }
    }
}
