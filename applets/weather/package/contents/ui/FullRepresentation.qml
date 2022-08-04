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
import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {
    id: fullRoot

    Layout.margins: PlasmaCore.Units.smallSpacing

    property alias generalModel: topPanel.generalModel
    property alias observationModel: topPanel.observationModel

    PlasmaExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: PlasmaCore.Units.largeSpacing
        // when not in panel, a configure button is already shown for needsConfiguration
        visible: root.needsConfiguration && (plasmoid.formFactor === PlasmaCore.Types.Vertical || plasmoid.formFactor === PlasmaCore.Types.Horizontal)
        iconName: "mark-location"
        text: i18n("Please set your location")
        helpfulAction: QQC2.Action {
            icon.name: "configure"
            text: i18n("Set location…")
            onTriggered: {
                plasmoid.action("configure").trigger();
            }
        }
    }

    Loader {
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true
        Layout.minimumWidth: topPanel.Layout.minimumWidth
        Layout.preferredHeight: topPanel.implicitHeight
        visible: !root.needsConfiguration

        active: activeFocus
        activeFocusOnTab: true
        asynchronous: true

        Accessible.name: topPanel.Accessible.name
        Accessible.description: topPanel.Accessible.description

        sourceComponent: PlasmaExtras.Highlight {
            hovered: true
        }

        TopPanel {
            id: topPanel
            anchors {
                left: parent.left
                right: parent.right
            }
        }
    }

    SwitchPanel {
        visible: !root.needsConfiguration
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true

        forecastViewTitle: generalModel.forecastTitle
        forecastModel: root.forecastModel
        detailsModel: root.detailsModel
        noticesModel: root.noticesModel
    }

    PlasmaComponents.Label {
        visible: !root.needsConfiguration
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
