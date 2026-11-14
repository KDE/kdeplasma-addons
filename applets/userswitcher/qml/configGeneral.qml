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

    Kirigami.Form {
        Kirigami.FormGroup {
            QtControls.ButtonGroup { id: nameGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:label", "Username style:")
                contentItem: QtControls.RadioButton {
                    id: showFullNameRadio
                    QtControls.ButtonGroup.group: nameGroup
                    text: i18nc("@option:radio", "Full name (if available)")
                    checked: root.cfg_showFullName
                    onClicked: if (checked) root.cfg_showFullName = true;
                }
            }

            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    QtControls.ButtonGroup.group: nameGroup
                    text: i18nc("@option:radio", "Login username")
                    checked: !root.cfg_showFullName
                    onClicked: if (checked) root.cfg_showFullName = false;
                }
            }
        }
        Kirigami.FormGroup {
            QtControls.ButtonGroup { id: layoutGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:label", "Show:")
                contentItem: QtControls.RadioButton {
                    id: showOnlyNameRadio

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
            }

            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
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
            }

            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
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
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:label", "Advanced:")
                contentItem: QtControls.CheckBox {
                    id: showTechnicalInfoCheck

                    text: i18nc("@option:check", "Show technical session information")
                }
            }
        }
    }
}
