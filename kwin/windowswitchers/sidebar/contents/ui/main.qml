/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kwin as KWin

KWin.TabBoxSwitcher {
    id: tabBox

    readonly property real screenFactor: screenGeometry.width / screenGeometry.height

    currentIndex: thumbnailListView.currentIndex

    PlasmaCore.Window {
        id: dialog
        borders: Application.layoutDirection === Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge
        visible: tabBox.visible
        flags: Qt.Popup | Qt.X11BypassWindowManagerHint
        x: screenGeometry.x + (Application.layoutDirection === Qt.RightToLeft ? screenGeometry.width - width : 0)
        y: screenGeometry.y

        width: mainItem.implicitWidth + leftPadding + rightPadding
        height: tabBox.screenGeometry.height

        mainItem: PlasmaComponents.ScrollView {
            id: dialogMainItem

            focus: true

            implicitWidth: tabBox.screenGeometry.width * 0.15

            LayoutMirroring.enabled: Application.layoutDirection === Qt.RightToLeft
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

