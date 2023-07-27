/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12 as QQC2

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.private.weather 1.0

ColumnLayout {
    id: fullRoot

    Layout.margins: Kirigami.Units.smallSpacing

    property alias generalModel: topPanel.generalModel
    property alias observationModel: topPanel.observationModel

    Layout.minimumWidth: Math.max(Kirigami.Units.gridUnit * 10, implicitWidth)
    Layout.minimumHeight: Math.max(Kirigami.Units.gridUnit * 10, implicitHeight)

    PlasmaExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: Kirigami.Units.gridUnit
        // when not in panel, a configure button is already shown for needsConfiguration
        visible: (root.status === Util.NeedsConfiguration) && (Plasmoid.formFactor === PlasmaCore.Types.Vertical || Plasmoid.formFactor === PlasmaCore.Types.Horizontal)
        iconName: "mark-location"
        text: i18n("Please set your location")
        helpfulAction: QQC2.Action {
            icon.name: "configure"
            text: i18n("Set location…")
            onTriggered: {
                Plasmoid.internalAction("configure").trigger();
            }
        }
    }

    PlasmaExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: Kirigami.Units.gridUnit
        Layout.maximumWidth: Kirigami.Units.gridUnit * 20
        visible: root.status === Util.Timeout
        iconName: "network-disconnect"
        text: {
            const sourceTokens = root.weatherSource.split("|");
            return i18n("Weather information retrieval for %1 timed out.", sourceTokens[2]);
        }
    }

    TopPanel {
        id: topPanel
        visible: root.status !== Util.NeedsConfiguration && root.status !== Util.Timeout

        Layout.fillWidth: true
    }

    SwitchPanel {
        id: switchPanel
        visible: root.status === Util.Normal
        Layout.fillWidth: true

        forecastViewTitle: generalModel.forecastTitle
        forecastViewNightRow: generalModel.forecastNightRow
        forecastModel: root.forecastModel
        detailsModel: root.detailsModel
        noticesModel: root.noticesModel
    }

    QQC2.Label {
        id: sourceLabel
        visible: root.status === Util.Normal
        readonly property string creditUrl: generalModel.creditUrl

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            cursorShape: !!parent.creditUrl ? Qt.PointingHandCursor : Qt.ArrowCursor
        }

        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignRight
        font {
            pointSize: Kirigami.Theme.smallFont.pointSize
            underline: !!creditUrl
        }
        linkColor : color
        opacity: 0.6
        textFormat: Text.StyledText

        text: {
            let result = generalModel.courtesy;
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
