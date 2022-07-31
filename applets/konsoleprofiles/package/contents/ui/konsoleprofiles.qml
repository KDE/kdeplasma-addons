/*
 *   SPDX-FileCopyrightText: 2011, 2012 Shaun Reich <shaun.reich@kdemail.net>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.profiles 1.0 as Profiles

FocusScope {
   id: konsoleProfiles

    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 11
    Plasmoid.switchHeight: PlasmaCore.Units.gridUnit * 9

    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 12
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 10

    Plasmoid.onExpandedChanged: {
        if (plasmoid.expanded) {
            view.forceActiveFocus();
        }
    }

    PlasmaCore.SortFilterModel {
        id: sortModel
        sortRole: "name"
        sortOrder: "AscendingOrder"
        sourceModel: Profiles.ProfilesModel {
            id: profilesModel
            appName: "konsole"
        }
    }

    Component.onCompleted: {
        plasmoid.popupIcon = "utilities-terminal";
    }

   PlasmaCore.Svg {
       id: lineSvg
       imagePath: "widgets/line"
    }

    Row {
        id: headerRow
        anchors { left: parent.left; right: parent.right }

        PlasmaCore.IconItem {
            id: appIcon
            source: "utilities-terminal"
            width: PlasmaCore.Units.iconSizes.medium
            height: PlasmaCore.Units.iconSizes.medium
        }

        PlasmaComponents3.Label {
            id: header
            text: i18nc("@title", "Konsole Profiles")
            horizontalAlignment: Text.AlignHCenter | Text.AlignVCenter
            width: parent.width - appIcon.width * 2
            height: parent.height
        }
    }

    PlasmaCore.SvgItem {
        id: separator

        anchors { left: headerRow.left; right: headerRow.right; top: headerRow.bottom }
        svg: lineSvg
        elementId: "horizontal-line"
        height: lineSvg.elementSize("horizontal-line").height
    }

    Text {
        id: textMetric
        visible: false
        // translated but not used, we just need length/height
        text: i18n("Arbitrary String Which Says Something")
    }

    PlasmaExtras.ScrollArea {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; top: separator.bottom; topMargin: PlasmaCore.Units.smallSpacing}

        ListView {
            id: view

            model: sortModel
            clip: true
            focus: true
            keyNavigationWraps: true

            delegate: PlasmaComponents3.ItemDelegate {
                id: listdelegate

                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: textMetric.paintedHeight * 2

                hoverEnabled: true
                text: model.name

                Accessible.role: Accessible.Button

                onClicked: {
                    openProfile();
                }

                onHoveredChanged: {
                    if (hovered) {
                        view.currentIndex = index;
                    }
                }

                function openProfile() {
                    /*var service = profilesSource.serviceForSource(model["DataEngineSource"])
                    var operation = service.operationDescription("open")
                    var  = service.startOperationCall(operation)*/
                    console.error(model.profileIdentifier)
                    profilesModel.openProfile(model.profileIdentifier)
                }
            }

            highlight: PlasmaExtras.Highlight {}

            highlightMoveDuration: PlasmaCore.Units.longDuration
            highlightMoveVelocity: 1
        }
    }
}
