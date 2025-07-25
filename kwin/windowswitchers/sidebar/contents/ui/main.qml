/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kwin 3.0 as KWin

KWin.TabBoxSwitcher {
    id: tabBox

    readonly property real screenFactor: screenGeometry.width / screenGeometry.height

    currentIndex: thumbnailListView.currentIndex

    PlasmaCore.Dialog {
        id: dialog
        location: Qt.application.layoutDirection === Qt.RightToLeft ? PlasmaCore.Types.RightEdge : PlasmaCore.Types.LeftEdge
        visible: tabBox.visible
        flags: Qt.Popup | Qt.X11BypassWindowManagerHint
        x: screenGeometry.x + (Qt.application.layoutDirection === Qt.RightToLeft ? screenGeometry.width - width : 0)
        y: screenGeometry.y

        mainItem: PlasmaComponents.ScrollView {
            id: dialogMainItem

            focus: true

            contentWidth: tabBox.screenGeometry.width * 0.15
            height: tabBox.screenGeometry.height - dialog.margins.top - dialog.margins.bottom

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            ListView {
                id: thumbnailListView
                focus: true
                model: tabBox.model
                spacing: Kirigami.Units.smallSpacing
                highlightMoveDuration: Kirigami.Units.longDuration
                highlightResizeDuration: 0

                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {
                        thumbnailListView.currentIndex = tabBox.currentIndex;
                        thumbnailListView.positionViewAtIndex(thumbnailListView.currentIndex, ListView.Contain)
                    }
                }

                delegate: MouseArea {
                    width: thumbnailListView.width
                    height: delegateColumn.implicitHeight + 2 * delegateColumn.anchors.margins
                    focus: ListView.isCurrentItem

                    Accessible.name: model.caption
                    Accessible.role: Accessible.ListItem

                    onClicked: {
                        if (tabBox.noModifierGrab) {
                            tabBox.model.activate(index);
                        } else {
                            thumbnailListView.currentIndex = index;
                        }
                    }

                    ColumnLayout {
                        id: delegateColumn
                        anchors {
                            left: parent.left
                            top: parent.top
                            right: parent.right
                            margins: Kirigami.Units.smallSpacing
                        }

                        spacing: Kirigami.Units.smallSpacing

                        Item {
                            Layout.fillWidth: true
                            implicitHeight: Math.round(delegateColumn.width / tabBox.screenFactor)

                            KWin.WindowThumbnail {
                                anchors.fill: parent
                                wId: windowId
                            }
                        }

                        RowLayout {
                            spacing: Kirigami.Units.smallSpacing
                            Layout.fillWidth: true

                            Kirigami.Icon {
                                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                                source: model.icon
                            }

                            Kirigami.Heading {
                                Layout.fillWidth: true
                                height: undefined
                                level: 4
                                text: model.caption
                                elide: Text.ElideRight
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                maximumLineCount: 2
                                lineHeight: 0.95
                                textFormat: Text.PlainText
                            }
                        }
                    }
                }

                highlight: PlasmaExtras.Highlight {}

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - Kirigami.Units.largeSpacing * 2
                    icon.source: "edit-none"
                    text: i18ndc("kwin", "@info:placeholder no entries in the task switcher", "No open windows")
                    visible: thumbnailListView.count === 0
                }
            }
        }

        onSceneGraphError: () => {
            // This slot is intentionally left blank, otherwise QtQuick may post a qFatal() message on a graphics reset.
        }
    }
}

