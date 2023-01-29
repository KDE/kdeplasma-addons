/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kwin 3.0 as KWin

KWin.TabBoxSwitcher {
    id: tabBox

    currentIndex: icons.currentIndex

    PlasmaCore.Dialog {
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        mainItem: ColumnLayout {
            id: dialogMainItem
            spacing: PlasmaCore.Units.smallSpacing * 2

            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.1, icons.implicitWidth), tabBox.screenGeometry.width * 0.9)

            IconTabBox {
                id: icons
                model: tabBox.model
                iconSize: PlasmaCore.Units.iconSizes.small
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: tabBox.screenGeometry.width * 0.9
            }

            PlasmaComponents3.Label {
                text: icons.currentItem ? icons.currentItem.caption : ""
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
                font.weight: Font.Bold
                Layout.fillWidth: true
            }

            Connections {
                target: tabBox
                function onCurrentIndexChanged() {
                    icons.currentIndex = tabBox.currentIndex;
                }
            }

            /*
            * Key navigation on outer item for two reasons:
            * @li we have to emit the change signal
            * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
            **/
            Keys.onPressed: {
                if (event.key == Qt.Key_Left) {
                    icons.decrementCurrentIndex();
                } else if (event.key == Qt.Key_Right) {
                    icons.incrementCurrentIndex();
                }
            }
        }
    }
}
