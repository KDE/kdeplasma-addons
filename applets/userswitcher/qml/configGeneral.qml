/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QtControls

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root
    property bool cfg_showFace
    property bool cfg_showName
    property bool cfg_showFullName
    property alias cfg_showTechnicalInfo: showTechnicalInfoCheck.checked

    Kirigami.FormLayout {
        QtControls.ButtonGroup {
            id: nameGroup
        }

        QtControls.RadioButton {
            id: showFullNameRadio

            Kirigami.FormData.label: i18nc("@title:label", "Username style:")

            QtControls.ButtonGroup.group: nameGroup
            text: i18nc("@option:radio", "Full name (if available)")
            checked: root.cfg_showFullName
            onClicked: if (checked) root.cfg_showFullName = true;
        }

        QtControls.RadioButton {
            QtControls.ButtonGroup.group: nameGroup
            text: i18nc("@option:radio", "Login username")
            checked: !root.cfg_showFullName
            onClicked: if (checked) root.cfg_showFullName = false;
        }


        Item {
            Kirigami.FormData.isSection: true
        }


        QtControls.ButtonGroup {
            id: layoutGroup
        }

        QtControls.RadioButton {
            id: showOnlyNameRadio

            Kirigami.FormData.label: i18nc("@title:label", "Show:")

            QtControls.ButtonGroup.group: layoutGroup
            text: i18nc("@option:radio", "Name")
            checked: root.cfg_showName && !root.cfg_showFace
            onClicked: {
                if (checked) {
                    root.cfg_showName = true;
                    root.cfg_showFace = false;
                }
            }
        }

        QtControls.RadioButton {
            id: showOnlyFaceRadio

            QtControls.ButtonGroup.group: layoutGroup
            text: i18nc("@option:radio", "User picture")
            checked: !root.cfg_showName && root.cfg_showFace
            onClicked: {
                if (checked) {
                    root.cfg_showName = false;
                    root.cfg_showFace = true;
                }
            }
        }

        QtControls.RadioButton {
            id: showBothRadio

            QtControls.ButtonGroup.group: layoutGroup
            text: i18nc("@option:radio", "Name and user picture")
            checked: root.cfg_showName && root.cfg_showFace
            onClicked: {
                if (checked) {
                    root.cfg_showName = true;
                    root.cfg_showFace = true;
                }
            }
        }


        Item {
            Kirigami.FormData.isSection: true
        }


        QtControls.CheckBox {
            id: showTechnicalInfoCheck

            Kirigami.FormData.label: i18nc("@title:label", "Advanced:")

            text: i18nc("@option:check", "Show technical session information")
        }
    }
}
