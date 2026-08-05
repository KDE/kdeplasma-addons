/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Item {
    id: root

    property var futureHours: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing * 2

    property int currentIndex: 0

    property int hoursPerPage: 0

    implicitWidth: forecast.implicitWidth
    implicitHeight: forecast.implicitHeight

    ColumnLayout {
        id: forecast

        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        HourlyForecastList {
            id: forecastList

            snapMode: ListView.SnapToItem

            Layout.fillWidth: true

            spacing: {
                if (!currentItem || root.hoursPerPage == 0) {
                    return 0;
                }
                const displayedItemCount = Math.min(count, root.hoursPerPage);
                const itemWidth = currentItem.implicitWidth;
                return displayedItemCount > 1 ? (width - displayedItemCount * itemWidth) / (displayedItemCount - 1) : 0;
            }

            metaData: root.metaData
            forecastModel: root.futureHours

            invalidUnit: root.invalidUnit
            displayTemperatureUnit: root.displayTemperatureUnit

            onWidthChanged: Qt.callLater(updatePageSize)
            Component.onCompleted: Qt.callLater(updatePageSize)

            // Calculate page size dynamically to fit as many forecasts as possible
            function updatePageSize() {
                if (!currentItem) {
                    root.hoursPerPage = 0;
                }
                root.hoursPerPage = Math.max(1, Math.floor((width + root.minimalSpacing) / (currentItem.implicitWidth + root.minimalSpacing)));
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            Layout.preferredHeight: Math.max(goFirstButton.implicitHeight, goPreviousButton.implicitHeight, goNextButton.implicitHeight, goLastButton.implicitHeight, indicator.implicitHeight)

            PlasmaComponents.ToolButton {
                id: goFirstButton
                icon.name: "go-first"
                text: i18n("Go to first day")
                display: PlasmaComponents.AbstractButton.IconOnly

                PlasmaComponents.ToolTip.text: text
                PlasmaComponents.ToolTip.visible: hovered || activeFocus
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay

                enabled: root.currentIndex > 0
                visible: indicator.visible
                onClicked: {
                    let newIndex = 0;
                    root.currentIndex = newIndex;
                    forecastList.positionViewAtIndex(newIndex, ListView.Beginning);
                }
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
            }

            PlasmaComponents.ToolButton {
                id: goPreviousButton
                icon.name: "go-previous"
                text: i18n("Go to previous day")
                display: PlasmaComponents.AbstractButton.IconOnly

                PlasmaComponents.ToolTip.text: text
                PlasmaComponents.ToolTip.visible: hovered || activeFocus
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay

                enabled: root.currentIndex > 0
                visible: indicator.visible
                onClicked: {
                    let newIndex = Math.max(0, root.currentIndex - root.hoursPerPage);
                    root.currentIndex = newIndex;
                    forecastList.positionViewAtIndex(newIndex, ListView.Beginning);
                }
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
            }

            PlasmaComponents.PageIndicator {
                id: indicator

                count: root.hoursPerPage > 0 ? Math.ceil(forecastList.count / root.hoursPerPage) : 0

                currentIndex: root.hoursPerPage > 0 ? Math.floor(root.currentIndex / root.hoursPerPage) : 0

                Layout.alignment: Qt.AlignHCenter

                visible: count > 1

                delegate: Rectangle {
                    width: Math.round(Kirigami.Units.gridUnit * 0.5)
                    height: width
                    radius: width / 2

                    color: index === indicator.currentIndex ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    opacity: index === indicator.currentIndex ? 1.0 : 0.4

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            let newIndex = Math.max(0, index * root.hoursPerPage);
                            root.currentIndex = newIndex;
                            forecastList.positionViewAtIndex(newIndex, ListView.Beginning);
                        }
                    }

                    Behavior on opacity {
                        OpacityAnimator {
                            duration: Kirigami.Units.shortDuration
                        }
                    }
                }
            }

            PlasmaComponents.ToolButton {
                id: goNextButton
                icon.name: "go-next"
                text: i18n("Go to next day")
                display: PlasmaComponents.AbstractButton.IconOnly

                ToolTip.text: text
                ToolTip.visible: hovered || activeFocus
                ToolTip.delay: Kirigami.Units.toolTipDelay

                visible: indicator.visible
                enabled: root.currentIndex + root.hoursPerPage < forecastList.count
                onClicked: {
                    let newIndex = Math.min(forecastList.count - 1, root.currentIndex + root.hoursPerPage);
                    root.currentIndex = newIndex;
                    forecastList.positionViewAtIndex(newIndex, ListView.Beginning);
                }
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
            }

            PlasmaComponents.ToolButton {
                id: goLastButton
                icon.name: "go-last"
                text: i18n("Go to last day")
                display: PlasmaComponents.AbstractButton.IconOnly

                PlasmaComponents.ToolTip.text: text
                PlasmaComponents.ToolTip.visible: hovered || activeFocus
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay

                visible: indicator.visible
                enabled: root.currentIndex + root.hoursPerPage < forecastList.count
                onClicked: {
                    let newIndex = Math.min(forecastList.count - 1, (indicator.count - 1) * root.hoursPerPage);
                    root.currentIndex = newIndex;
                    forecastList.positionViewAtIndex(newIndex, ListView.Beginning);
                }
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
            }
        }
    }
}
