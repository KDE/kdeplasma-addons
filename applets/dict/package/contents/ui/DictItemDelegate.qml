/*
    SPDX-FileCopyrightText: 2020 Ismael Asensio <isma.af@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

// External item required to make Kirigami.ListItemDragHandle work
Item {
    id: delegate

    property ListView view

    implicitHeight : dictItem.implicitHeight

    signal removed(int index)
    signal moveRequested(int oldIndex, int newIndex)

    Kirigami.SwipeListItem {
        id: dictItem
        hoverEnabled: false

        RowLayout {
            Kirigami.ListItemDragHandle {
                listItem: dictItem
                listView: delegate.view
                onMoveRequested: {
                    delegate.moveRequested(oldIndex, newIndex);
                }
            }

            Kirigami.BasicListItem {
                Layout.fillWidth: true

                hoverEnabled: false
                separatorVisible: false

                label: model.id
                subtitle: model.description
            }
        }

        actions: [
            Kirigami.Action {
                text: i18n("Delete")
                iconName: "entry-delete"
                onTriggered: {
                    delegate.removed(index);
                }
            }
        ]
    }
}
