/*
 *  SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.5
import org.kde.kirigami 2.19 as Kirigami
import org.kde.plasma.private.dict 1.0

Page {
    id: page
    property string cfg_dictionary: ""

    // "root" is from ConfigurationAppletPage.qml
    implicitHeight: root.availableHeight

    DictionariesModel {
        id: dictionariesModel
    }

    ScrollView {
        anchors.fill: parent
        Component.onCompleted: background.visible = true;

        ListView {
            id: listView
            model: dictionariesModel
            reuseItems: true

            section {
                criteria: ViewSection.FirstCharacter
                property: "id"
                delegate: Kirigami.ListSectionHeader {
                    label: section
                }
            }

            delegate: Kirigami.BasicListItem {
                width: listView.width

                bold: page.cfg_dictionary == model.id
                highlighted: bold

                icon: undefined

                label: model.id
                subtitle: model.description

                onClicked: page.cfg_dictionary = model.id
            }
        }
    }

    Loader {
        active: dictionariesModel.loading || listView.count === 0
        asynchronous: true

        anchors.centerIn: parent
        visible: active

        sourceComponent: dictionariesModel.loading ? loadingPlaceHolder : errorPlaceHolder

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
    }
}
