/*
    SPDX-FileCopyrightText: 2013 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigami.dialogs as KDialogs
import org.kde.kitemmodels as KItemModels

Kirigami.Dialog {
    id: dictDialog

    readonly property alias view: dialogListView
    property alias dictionaryModel: proxyModel.sourceModel

    title: i18n("Add More Dictionaries")
    implicitWidth: dialogListView.implicitWidth
    implicitHeight: Math.round(dictDialog.parent.height * 0.8)
    padding: 0

    onOpened: {
        filter.text = "";
        filter.forceActiveFocus()
    }

    header: KDialogs.DialogHeader {
        dialog: dictDialog
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            KDialogs.DialogHeaderTopContent {
                dialog: dictDialog
            }

            Kirigami.SearchField {
                id: filter
                Layout.fillWidth: true
            }
        }
    }

    ListView {
        id: dialogListView
        focus: true // keyboard navigation
        activeFocusOnTab: true // keyboard navigation
        implicitWidth: Kirigami.Units.gridUnit * 25


        reuseItems: true

        model: KItemModels.KSortFilterProxyModel {
            id: proxyModel
            filterRoleName: "EditRole" // id
            filterString: filter.text
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: QQC2.CheckDelegate {
            id: checkbox

            required property string id
            required property string description
            required property int index
            required property var model // checked shadows CheckDelegate's'

            width: dialogListView.width
            focus: true // keyboard navigation
            text: `${id} (${description})`
            checked: model.checked
            onToggled: {
                model.checked = checked;
                dialogListView.currentIndex = index; // highlight
                dialogListView.forceActiveFocus(); // keyboard navigation
            }
            highlighted: ListView.isCurrentItem
        }
    }
}
