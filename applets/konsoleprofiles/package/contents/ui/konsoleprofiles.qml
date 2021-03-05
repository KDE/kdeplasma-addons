/*
 *   SPDX-FileCopyrightText: 2011, 2012 Shaun Reich <shaun.reich@kdemail.net>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents // for Highlight
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

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

    PlasmaCore.DataSource {
        id: profilesSource
        engine: "org.kde.konsoleprofiles"
        onSourceAdded: connectSource(source)
        onSourceRemoved: disconnectSource(source)

        Component.onCompleted: connectedSources = sources
    }

    PlasmaCore.SortFilterModel {
        id: profilesModel
        sortRole: "prettyName"
        sortOrder: "AscendingOrder"
        sourceModel: PlasmaCore.DataModel {
            dataSource: profilesSource
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


            model: profilesModel
            clip: true
            focus: true
            keyNavigationWraps: true

            delegate: Item {
                id: listdelegate
                height: textMetric.paintedHeight * 2

                anchors {
                    left: parent.left
                    right: parent.right
                }

                function openProfile() {
                    var service = profilesSource.serviceForSource(model["DataEngineSource"])
                    var operation = service.operationDescription("open")
                    var job = service.startOperationCall(operation)
                }

                PlasmaComponents3.Label {
                    id: profileText

                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        right: parent.right
                        leftMargin: 10
                        rightMargin: 10
                    }

                    verticalAlignment: Text.AlignVCenter
                    text: model.prettyName
                    elide: Text.ElideRight
                }

                MouseArea {
                    height: parent.height + 15
                    anchors { left: parent.left; right: parent.right;}
                    hoverEnabled: true

                    onClicked: {
                        openProfile();
                    }

                    onEntered: {
                        view.currentIndex = index;
                    }
                }

                Keys.onPressed: {
                    if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return)
                        openProfile();
                }
            }

            highlight: PlasmaComponents.Highlight {
                hover: true
            }

            highlightMoveDuration: PlasmaCore.Units.longDuration
            highlightMoveVelocity: 1
        }
    }
}
