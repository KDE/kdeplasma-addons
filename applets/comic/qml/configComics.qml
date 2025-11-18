/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts as Layouts

import org.kde.newstuff as NewStuff
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: root

    property list<string> cfg_tabIdentifier
    property alias cfg_checkNewComicStripsIntervall: checkNewComicStripsInterval.value
    property alias cfg_maxComicLimit: maxComicLimit.value

    actions: [
        NewStuff.Action {
            id: newStuffAction
            text: i18nc("@action:button", "Get New…")  // qmllint disable unqualified
            configFile: "comic.knsrc"
            onEntryEvent: function(entry, event) {
                if (event == 1) {
                    Plasmoid.loadProviders()
                }
            }
        }
    ]

    // HACK: get ScrollViewKCM to draw the separator line while there's no proper API to do so
    function __headerSeparatorVisible(): bool {
        return true
    }

    extraFooterTopPadding: true

    view: ListView {
        id: comicListView
        model: Plasmoid.availableComicsModel
        delegate: Kirigami.CheckSubtitleDelegate {
            id: checkdelegate

            required property var model
            required property string decoration
            required property string plugin

            width: comicListView.width
            text: model.display // can't be required as it conflicts with AbstractButton
            icon.source: decoration
            checked: model.checked // can't be required as it conflicts with AbstractButton
            onCheckedChanged: {
                if (model.checked != checked) {
                    model.checked = checked
                    root.cfg_tabIdentifier = Plasmoid.availableComicsModel.checkedProviders()
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: comicListView.count < 1
            icon.name: "folder-comic-symbolic"
            text: i18nc("@info placeholdermessage if comic provider list empty", "No comics installed") // qmllint disable unqualified
            helpfulAction: NewStuff.Action { // not reusing toolbar action as the text feels awkward here
                text: i18nc("@action:button", "Get Comics…") // qmllint disable unqualified
                configFile: "comic.knsrc"
                onEntryEvent: function(entry, event) {
                    if (event == 1) {
                        Plasmoid.loadProviders()
                    }
                }
            }

        }
    }

    footer: Kirigami.FormLayout {
        Layouts.RowLayout {
            Kirigami.FormData.label: i18nc("@label:spinbox", "Check for new comics every:") // qmllint disable unqualified

            Controls.SpinBox {
                id: checkNewComicStripsInterval
                from: 0
                to: 999
                stepSize: 1
            }

            Controls.Label {
                text: i18ncp("@item:valuesuffix spacing to number + unit (minutes)", "minute", "minutes") // qmllint disable unqualified
                textFormat: Text.PlainText
            }
        }

        Layouts.RowLayout {
            Kirigami.FormData.label: i18nc("@label:spinbox", "Keep local archive:") // qmllint disable unqualified

            Controls.SpinBox {
                id: maxComicLimit
                stepSize: 1
            }

            Controls.Label {
                text: i18ncp("@item:valuesuffix spacing to number + unit", "images per comic", "images per comic") // qmllint disable unqualified
                textFormat: Text.PlainText
            }
        }
    }
}
