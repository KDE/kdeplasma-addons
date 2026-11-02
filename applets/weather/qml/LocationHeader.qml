/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore

RowLayout {
    id: root

    property var station: null
    property var warnings: null

    signal openWarnings

    Layout.fillWidth: true
    Layout.minimumWidth: Kirigami.Units.gridUnit * 10

    spacing: Kirigami.Units.smallSpacing

    Kirigami.Heading {
        id: locationLabel

        Layout.fillWidth: true

        visible: !!root.station?.place

        elide: Text.ElideRight
        text: visible ? root.station.place : ""
        textFormat: Text.PlainText

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: locationLabel.text
            active: locationLabel.truncated
        }
    }

    PlasmaComponents.Button {
        Layout.alignment: Qt.AlignRight
        text: !!root.warnings ? i18ncp("@title:tab %1 is the number of weather notices (alerts, warnings, watches, ...) issued", "%1 Notice", "%1 Notices", root.warnings.count) : ""
        icon.name: !!root.warnings && root.warnings.maxPriorityCount >= 2 ? 'data-warning-symbolic' : 'data-information-symbolic'
        visible: !!root.warnings && root.warnings.count !== 0
        flat: true

        onClicked: root.openWarnings()
    }
}
