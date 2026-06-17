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

ColumnLayout {
    id: root

    property var futureHours: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing * 2

    readonly property int pageSize: Math.max(1, Math.floor((width + root.minimalSpacing) / (forecastList.itemWidth + root.minimalSpacing)))

    property int currentIndex: 0

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        PlasmaComponents.ToolButton {
            Layout.fillHeight: true
            icon.name: "go-previous"
            enabled: root.currentIndex > 0
            onClicked: {
                let newIndex = Math.max(0, root.currentIndex - root.pageSize);
                root.currentIndex = newIndex;
                forecastList.positionViewAtIndex(newIndex, ListView.Beginning);
            }
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }

        HourlyForecastList {
            id: forecastList

            Layout.fillWidth: true
            Layout.fillHeight: true

            snapMode: ListView.SnapToItem

            readonly property int displayedItemCount: Math.min(count, root.pageSize)

            readonly property real itemWidth: currentItem.implicitWidth

            spacing: displayedItemCount > 1 ? (width - displayedItemCount * itemWidth) / (displayedItemCount - 1) : 0

            metaData: root.metaData
            forecastModel: root.futureHours

            invalidUnit: root.invalidUnit
            displayTemperatureUnit: root.displayTemperatureUnit
        }

        PlasmaComponents.ToolButton {
            Layout.fillHeight: true
            icon.name: "go-next"
            enabled: root.currentIndex + root.pageSize < forecastList.count
            onClicked: {
                let newIndex = Math.min(forecastList.count - 1, root.currentIndex + root.pageSize);
                root.currentIndex = newIndex;
                forecastList.positionViewAtIndex(newIndex, ListView.Beginning);
            }
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }
    }

    PlasmaComponents.PageIndicator {
        id: indicator
        count: Math.ceil(forecastList.count / root.pageSize)

        currentIndex: Math.floor(root.currentIndex / root.pageSize)

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
                    let newIndex = Math.max(0, index * root.pageSize);
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
}
