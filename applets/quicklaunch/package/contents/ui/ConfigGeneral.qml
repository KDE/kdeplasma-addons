/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5 as QQC2

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    readonly property bool vertical: plasmoid.formFactor == PlasmaCore.Types.Vertical || (plasmoid.formFactor == PlasmaCore.Types.Planar && plasmoid.height > plasmoid.width)

    property alias cfg_maxSectionCount: maxSectionCount.value
    property alias cfg_showLauncherNames: showLauncherNames.checked
    property alias cfg_enablePopup: enablePopup.checked
    property alias cfg_title: title.text

    Kirigami.FormLayout {
        QQC2.SpinBox {
            id: maxSectionCount

            Kirigami.FormData.label: vertical ? i18nc("@label:spinbox", "Maximum columns:") : i18nc("@label:spinbox", "Maximum rows:")

            from: 1
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: showLauncherNames

            Kirigami.FormData.label: i18nc("@title:group", "Appearance:")

            text: i18nc("@option:check", "Show launcher names")
        }

        QQC2.CheckBox {
            id: enablePopup
            text: i18nc("@option:check", "Enable popup")
        }


        Item {
            Kirigami.FormData.isSection: true
        }


        RowLayout {
            Kirigami.FormData.label: i18nc("@title:group", "Title:")
            Layout.fillWidth: true

            visible: plasmoid.formFactor == PlasmaCore.Types.Planar

            QQC2.CheckBox {
                id: showTitle
                checked: title.length
                text: i18nc("@option:check", "Show:")

                onClicked: {
                    if (checked) {
                        title.forceActiveFocus();
                    } else {
                        title.text = "";
                    }
                }
            }

            Kirigami.ActionTextField {
                id: title
                enabled: showTitle.checked

                Layout.fillWidth: true
                placeholderText: i18nc("@info:placeholder", "Custom title")

                rightActions: [
                    Kirigami.Action {
                        icon.name: "edit-clear"
                        visible: title.text.length !== 0
                        onTriggered: title.text = "";
                    }
                ]
            }
        }
    }
}
