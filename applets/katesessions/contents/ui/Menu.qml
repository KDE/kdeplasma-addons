/*
    SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    Based on the clipboard applet:
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as QQC2
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.components as PlasmaComponents3

PlasmaComponents3.ScrollView {
    id: menu
    property alias view: menuListView
    property alias model: menuListView.model
    signal itemSelected(string profileIdentifier)

    ListView {
        id: menuListView
        focus: true

        boundsBehavior: Flickable.StopAtBounds
        interactive: contentHeight > height
        highlight: PlasmaExtras.Highlight {
            anchors.bottomMargin: -listMargins.bottom
            y: 1
        }
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        currentIndex: -1
        onActiveFocusChanged: if (activeFocus && currentIndex == -1) currentIndex = 0
        Keys.onReturnPressed: (currentItem as KateSessionsItemDelegate).clicked()
        Keys.onEnterPressed: Keys.returnPressed()

        delegate: KateSessionsItemDelegate {
            width: menuListView.width

            onItemSelected: (profileIdentifier) => {
                menu.itemSelected(profileIdentifier)
                menuListView.currentIndex=-1
            }

            onHoveredChanged: () => {
                if (hovered)  {
                    menuListView.currentIndex = index
                } else {
                    if (menuListView.currentIndex == index) menuListView.currentIndex = -1
                }
            }
        }
    }
}
