/*
 *  Copyright 2017 David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
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
            implicitWidth: units.gridUnit * 40
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
        zoomFactor: units.devicePixelRatio
        profile: dict.webProfile
    }

}
