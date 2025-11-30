/*
 *  SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.plasma.private.dict

KCM.ScrollViewKCM {

    property string cfg_dictionary: ""

    actions: [
        Kirigami.Action {
            enabled: sheet.view.count > 0
            text: i18nc("@action:button Add more dictionaries", "Add More…")
            icon.name: "list-add"
            onTriggered: sheet.open()
        }
    ]

    DictionariesModel {
        id: dictionariesModel
        Component.onCompleted: enabledDicts = cfg_dictionary
        onEnabledDictsChanged: cfg_dictionary = enabledDicts
    }

    AvailableDictSheet {
        id: sheet
    }

    view: ListView {
        id: listView

        model: dictionariesModel.enabledDictModel
        reuseItems: true

        displaced: Transition {
            NumberAnimation {
                properties: "y"
                duration: Kirigami.Units.longDuration
            }
        }

        delegate: DictItemDelegate {
            width: listView.width
            view: listView

            onMoveRequested: (oldIndex, newIndex) => {
                dictionariesModel.move(oldIndex, newIndex);
            }
            onRemoved: index => {
                dictionariesModel.setDisabled(index);
            }
        }

        Loader {
            active: dictionariesModel.loading || sheet.view.count === 0 || listView.count === 0
            asynchronous: true

            anchors.centerIn: parent
            visible: active

            sourceComponent: dictionariesModel.loading ? loadingPlaceHolder : (sheet.view.count === 0 ? errorPlaceHolder : emptyPlaceholder)

            Component {
                id: loadingPlaceHolder

                Kirigami.LoadingPlaceholder {
                    anchors.centerIn: parent
                }
            }

            Component {
                id: errorPlaceHolder

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: root.width - (Kirigami.Units.largeSpacing * 4)
                    icon.name: "network-disconnect"
                    text: i18n("Unable to load dictionary list")
                    explanation: i18nc("%2 human-readable error string", "Error code: %1 (%2)", dictionariesModel.errorCode, dictionariesModel.errorString)
                }
            }

            Component {
                id: emptyPlaceholder

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: root.width - (Kirigami.Units.largeSpacing * 4)
                    icon.name: "edit-none"
                    text: i18n("No dictionaries")
                }
            }
        }
    }
}
