/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.components as PlasmaComponents

ColumnLayout {
    id: root

    required property var forecastModel
    required property var detailsModel
    required property var noticesModel

    required property bool forecastViewNightRow
    required property string forecastViewTitle

    readonly property var pagesModel: {
        const pages = [{
            title: root.forecastViewTitle,
            view: forecastView,
        }]
        if (root.detailsModel && root.detailsModel.length > 0) {
            pages.push({
                title: i18nc("@title:tab", "Details"),
                view: detailsView,
            })
        }
        if (root.noticesModel && root.noticesModel.length > 0
                && (root.noticesModel[0].length > 0 || root.noticesModel[1].length > 0)) {
            pages.push({
                title: i18nc("@title:tab", "Notices"),
                view: noticesView,
            })
        }
        return pages
    }

    PlasmaComponents.TabBar {
        id: tabBar

        Layout.fillWidth: true
        visible: root.pagesModel.length > 1

        Repeater {
            model: root.pagesModel
            delegate: PlasmaComponents.TabButton {
                text: modelData.title
            }
        }

        onCurrentIndexChanged: {
            swipeView.setCurrentIndex(currentIndex);
        }
    }

    QQC2.SwipeView {
        id: swipeView

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.minimumWidth: contentChildren.reduce((acc, loader) => Math.max(loader.implicitWidth, acc), 0)
        Layout.minimumHeight: contentChildren.reduce((acc, loader) => Math.max(loader.implicitHeight, acc), 0)
        clip: true // previous/next views are prepared outside of view, do not render them

        currentIndex: tabBar.currentIndex

        Repeater {
            model: root.pagesModel
            delegate: Loader {
                sourceComponent: modelData.view
            }
        }

        onCurrentIndexChanged: {
            tabBar.setCurrentIndex(currentIndex);
        }
    }

    Component {
        id: forecastView
        ForecastView {
            model: root.forecastModel
            showNightRow: root.forecastViewNightRow
        }
    }

    Component {
        id: detailsView
        DetailsView {
            model: root.detailsModel
        }
    }

    Component {
        id: noticesView
        NoticesView {
            model: root.noticesModel
        }
    }
}
