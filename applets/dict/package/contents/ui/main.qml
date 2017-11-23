import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import QtWebEngine 1.1

import org.kde.plasma.private.dict 1.0

ColumnLayout {

    DictObject {
        id: dict
        onSearchInProgress: web.loadHtml(i18n("Looking up definition..."));
        onDefinitionFound: web.loadHtml(html);
    }

    PlasmaComponents.TextField {
        id: input
        placeholderText: i18n("Enter word to define here")
        implicitWidth: units.gridUnit * 40
        Layout.fillWidth: true
        onAccepted: {
            if (input.text === "") {
                web.visible = false;
            } else {
                web.visible = true;
                dict.lookup(input.text);
            }
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
