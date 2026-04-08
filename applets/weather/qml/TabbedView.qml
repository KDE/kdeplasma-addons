/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

ColumnLayout {
    id: root

    property bool showHourlyForecast: false
    property string hourlySubHeaderText: ""
    property var hourlyComponent: null

    property var dayComponent: null
    property bool showDayForecast: false

    readonly property alias hourlyItem: tabbedHourlyLoader.item
    readonly property alias dayItem: tabbedDayLoader.item

    PlasmaComponents.TabBar {
        id: tabBar

        Layout.fillWidth: true

        visible: root.showHourlyForecast && root.showDayForecast

        PlasmaComponents.TabButton {
            contentItem: ColumnLayout {
                spacing: 0
                PlasmaComponents.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: i18nc("@title:tab", "Hourly")
                }

                PlasmaComponents.Label {
                    Layout.fillWidth: true
                    font: Kirigami.Theme.smallFont
                    opacity: 0.75
                    text: root.hourlySubHeaderText
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }
            }
        }

        PlasmaComponents.TabButton {
            text: i18ncp("@title:tab", "%1 Day", "%1 Days", tabbedDayLoader.item?.daysCount ?? 0)
        }

        onCurrentIndexChanged: {
            swipeView.setCurrentIndex(currentIndex);
        }
    }

    QQC2.SwipeView {
        id: swipeView

        Layout.fillWidth: true
        Layout.minimumWidth: contentChildren.reduce((acc, loader) => Math.max(loader.status == Loader.Ready ? loader.implicitWidth : 0, acc), 0)
        Layout.minimumHeight: contentChildren.reduce((acc, loader) => Math.max(loader.status == Loader.Ready ? loader.implicitHeight : 0, acc), 0)
        clip: true // previous/next views are prepared outside of view, do not render them

        currentIndex: tabBar.currentIndex

        Loader {
            id: tabbedHourlyLoader
            active: root.showHourlyForecast
            visible: status == Loader.Ready
            sourceComponent: root.hourlyComponent
        }

        Loader {
            id: tabbedDayLoader
            active: root.showDayForecast
            visible: status == Loader.Ready
            sourceComponent: root.dayComponent
        }

        onCurrentIndexChanged: {
            tabBar.setCurrentIndex(currentIndex);
        }
    }
}
