/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {
    id: fullRoot

    Layout.margins: PlasmaCore.Units.smallSpacing

    property alias generalModel: topPanel.generalModel
    property alias observationModel: topPanel.observationModel

    TopPanel {
        id: topPanel

        Layout.fillWidth: true
    }

    SwitchPanel {
        Layout.fillWidth: true

        forecastViewTitle: generalModel.forecastTitle
        forecastModel: root.forecastModel
        detailsModel: root.detailsModel
        noticesModel: root.noticesModel
    }

    PlasmaComponents.Label {
        readonly property string creditUrl: generalModel.creditUrl

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignRight
        font {
            pointSize: PlasmaCore.Theme.smallestFont.pointSize
            underline: !!creditUrl
        }
        linkColor : color
        opacity: 0.6
        textFormat: Text.StyledText

        text: {
            var result = generalModel.courtesy;
            if (creditUrl) {
                result = "<a href=\"" + creditUrl + "\">" + result + "</a>";
            }
            return result;
        }

        onLinkActivated: {
            Qt.openUrlExternally(link);
        }
    }
}
