/*
 *  SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import QtWebEngine 1.1

import org.kde.plasma.private.dict 1.0

ColumnLayout {

    DictObject {
        id: dict
        selectedDictionary: plasmoid.configuration.dictionary
        onSearchInProgress: web.loadHtml(i18n("Looking up definition..."));
        onDefinitionFound: web.loadHtml(html);
    }

    RowLayout {
        Layout.fillWidth: true
        PlasmaComponents3.TextField {
            id: input
            placeholderText: i18nc("@info:placeholder", "Enter word to define here")
            implicitWidth: PlasmaCore.Units.gridUnit * 40
            onAccepted: {
                if (input.text === "") {
                    web.visible = false;
                } else {
                    web.visible = true;
                    dict.lookup(input.text);
                }
            }
        }
        PlasmaComponents3.Button {
            icon.name: "configure"
            onClicked: plasmoid.action("configure").trigger();
        }
    }

    WebEngineView {
        id: web
        visible: false
        Layout.fillWidth: true
        //Layout.fillHeight: true
        Layout.preferredHeight: 400
        zoomFactor: PlasmaCore.Units.devicePixelRatio
        profile: dict.webProfile
    }

}
