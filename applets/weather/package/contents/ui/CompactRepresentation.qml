/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.workspace.components 2.0 as WorkspaceComponents

Loader {
    id: compactRoot

    property var generalModel
    property var observationModel

    readonly property bool vertical: (Plasmoid.formFactor == PlasmaCore.Types.Vertical)
    readonly property bool showTemperature: Plasmoid.configuration.showTemperatureInCompactMode
    readonly property bool useBadge: Plasmoid.configuration.showTemperatureInBadge || Plasmoid.nativeInterface.needsToBeSquare

    sourceComponent: (showTemperature && !useBadge) ? iconAndTextComponent : iconComponent
    Layout.fillWidth: compactRoot.vertical
    Layout.fillHeight: !compactRoot.vertical
    Layout.minimumWidth: item.Layout.minimumWidth
    Layout.minimumHeight: item.Layout.minimumHeight

    MouseArea {
        id: compactMouseArea
        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            Plasmoid.expanded = !Plasmoid.expanded;
        }
    }

    Component {
        id: iconComponent

        PlasmaCore.IconItem {
            readonly property int minIconSize: Math.max((compactRoot.vertical ? compactRoot.width : compactRoot.height), PlasmaCore.Units.iconSizes.small)

            source: generalModel.currentConditionIconName
            active: compactMouseArea.containsMouse
            // reset implicit size, so layout in free dimension does not stop at the default one
            implicitWidth: PlasmaCore.Units.iconSizes.small
            implicitHeight: PlasmaCore.Units.iconSizes.small
            Layout.minimumWidth: compactRoot.vertical ? PlasmaCore.Units.iconSizes.small : minIconSize
            Layout.minimumHeight: compactRoot.vertical ? minIconSize : PlasmaCore.Units.iconSizes.small

            WorkspaceComponents.BadgeOverlay {
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                visible: showTemperature && useBadge && text.length > 0

                text: observationModel.temperature
                icon: parent
            }
        }
    }

    Component {
        id: iconAndTextComponent

        IconAndTextItem {
            vertical: compactRoot.vertical
            iconSource: generalModel.currentConditionIconName
            active: compactMouseArea.containsMouse
            text: observationModel.temperature
        }
    }
}
