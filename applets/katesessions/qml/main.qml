/*
    SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    Based on the clipboard applet:
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras

import org.kde.plasma.private.profiles

PlasmoidItem {
    id: main

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    width: (Plasmoid.formFactor==PlasmaCore.Types.Planar)? Kirigami.Units.gridUnit * 14 : undefined
    height: (Plasmoid.formFactor==PlasmaCore.Types.Planar)? Kirigami.Units.gridUnit * 16: undefined

    switchWidth: Kirigami.Units.gridUnit * 11
    switchHeight: Kirigami.Units.gridUnit * 11
    Plasmoid.status: PlasmaCore.Types.ActiveStatus
    Plasmoid.icon: inPanel ? "kate-symbolic" : "kate"
    toolTipMainText: i18n("Kate Sessions")

    Component.onCompleted: {
        plasmoid.removeInternalAction("configure");
    }

    fullRepresentation: PlasmaComponents3.Page {

        id: dialogItem
        Layout.minimumWidth: Kirigami.Units.gridUnit * 12
        Layout.minimumHeight: Kirigami.Units.gridUnit * 12
        Layout.preferredWidth: Kirigami.Units.gridUnit * 16
        Layout.preferredHeight: Kirigami.Units.gridUnit * 24

        focus: true
        header: PlasmaExtras.PlasmoidHeading {
            PlasmaComponents3.TextField {
                id: filter
                placeholderText: i18n("Search…")
                clearButtonShown: true
                anchors.fill: parent

                onActiveFocusChanged: if (activeFocus) sessionsMenu.view.currentIndex = -1

                KeyNavigation.down: sessionsMenu.view
                KeyNavigation.tab: sessionsMenu.view
                Keys.onReturnPressed: {
                    (sessionsMenu.view.itemAtIndex(0) as KateSessionsItemDelegate)?.clicked()
                }
                Keys.onEnterPressed: Keys.returnPressed()
                Keys.onEscapePressed: clearOrHide()

                function clearOrHide() {
                    if (text == "") {
                        main.expanded = false;
                    } else {
                        text = "";
                    }
                }
            }
        }

        property alias listMargins: listItemSvg.margins

        KSvg.FrameSvgItem {
            id : listItemSvg
            imagePath: "widgets/listitem"
            prefix: "normal"
            visible: false
        }

        Keys.onEscapePressed: filter.clearOrHide()
        Keys.onTabPressed: filter.forceActiveFocus(Qt.TabFocusReason)
        Keys.onPressed: (event) => {
            // forward key to filter
            // filter.text += event.text will break if the key is backspace
            if (event.key == Qt.Key_Backspace && filter.text == "") {
                return;
            }
            if (event.text != "" && !filter.activeFocus) {
                sessionsMenu.view.currentIndex = -1
                if (event.text == "v" && event.modifiers & Qt.ControlModifier) {
                    filter.paste();
                } else {
                    filter.text = "";
                    filter.text += event.text;
                }
                filter.forceActiveFocus(Qt.ShortcutFocusReason);
                event.accepted = true;
            }
        }
        ColumnLayout {
            anchors.fill: parent
            Menu {
                id: sessionsMenu
                model: KItemModels.KSortFilterProxyModel {
                    sourceModel: ProfilesModel {
                        id: model
                        appName: "kate"
                    }
                    filterRoleName: "name"
                    filterString: filter.text
                    filterCaseSensitivity: Qt.CaseInsensitive
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                onItemSelected: function (profileIdentifier) {
                    model.openProfile(profileIdentifier)
                    main.expanded = false;
                }
            }
        }
        Connections {
            target: main
            function onExpandedChanged() {
                if (main.expanded) {
                    sessionsMenu.view.currentIndex = -1
                    filter.forceActiveFocus(Qt.PopupFocusReason)
                }
            }
        }
    }
}
