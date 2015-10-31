/*
 * Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0

ColumnLayout {
    id: appearancePage

    property bool cfg_showFace
    property bool cfg_showName
    property bool cfg_showFullName
    property alias cfg_showTechnicalInfo: showTechnicalInfoCheck.checked

    QtControls.GroupBox {
        title: i18n("User name display")
        flat: true

        QtControls.ExclusiveGroup {
            id: nameEg
            onCurrentChanged: cfg_showFullName = (current === showFullNameRadio)
        }

        ColumnLayout {
            QtControls.RadioButton {
                id: showFullNameRadio
                Layout.fillWidth: true
                exclusiveGroup: nameEg
                text: i18n("Show full name (if available)")
                checked: cfg_showFullName
            }

            QtControls.RadioButton {
                Layout.fillWidth: true
                exclusiveGroup: nameEg
                text: i18n("Show login username")
                checked: !cfg_showFullName
            }
        }
    }

    QtControls.GroupBox {
        title: i18n("Layout")
        flat: true

        QtControls.ExclusiveGroup {
            id: layoutEg
            onCurrentChanged: {
                cfg_showName = (current === showOnlyNameRadio || current === showBothRadio)
                cfg_showFace = (current === showOnlyFaceRadio || current === showBothRadio)
            }
        }

        ColumnLayout {
            QtControls.RadioButton {
                id: showOnlyNameRadio
                Layout.fillWidth: true
                exclusiveGroup: layoutEg
                text: i18n("Show only name")
                checked: cfg_showName && !cfg_showFace
            }

            QtControls.RadioButton {
                id: showOnlyFaceRadio
                Layout.fillWidth: true
                exclusiveGroup: layoutEg
                text: i18n("Show only avatar")
                checked: !cfg_showName && cfg_showFace
            }

            QtControls.RadioButton {
                id: showBothRadio
                Layout.fillWidth: true
                exclusiveGroup: layoutEg
                text: i18n("Show both avatar and name")
                checked: cfg_showName && cfg_showFace
            }

            QtControls.CheckBox {
                id: showTechnicalInfoCheck
                text: i18n("Show technical information about sessions")
            }
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
