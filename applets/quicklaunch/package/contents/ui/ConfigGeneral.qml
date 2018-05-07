/*
 *  Copyright 2015 David Rosca <nowrep@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0
import org.kde.plasma.core 2.0 as PlasmaCore

ColumnLayout {

    readonly property bool vertical: plasmoid.formFactor == PlasmaCore.Types.Vertical || (plasmoid.formFactor == PlasmaCore.Types.Planar && plasmoid.height > plasmoid.width)

    property alias cfg_maxSectionCount: maxSectionCount.value
    property alias cfg_showLauncherNames: showLauncherNames.checked
    property alias cfg_enablePopup: enablePopup.checked
    property alias cfg_title: title.text

    GroupBox {
        Layout.fillWidth: true
        flat: true

        title: i18nc("@title:group", "Arrangement")

        RowLayout {
            Label {
                text: vertical ? i18nc("@label:spinbox", "Maximum columns:") : i18nc("@label:spinbox", "Maximum rows:")
            }

            SpinBox {
                id: maxSectionCount
                minimumValue: 1
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        flat: true

        title: i18nc("@title:group", "Appearance")

        ColumnLayout {
            CheckBox {
                id: showLauncherNames
                text: i18nc("@option:check", "Show launcher names")
            }

            CheckBox {
                id: enablePopup
                text: i18nc("@option:check", "Enable popup")
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        flat: true

        title: i18nc("@title:group", "Title")
        visible: plasmoid.formFactor == PlasmaCore.Types.Planar

        RowLayout {
            anchors.fill: parent
            CheckBox {
                id: showTitle
                checked: title.activeFocus || title.length
                text: i18nc("@option:check", "Show title")

                onClicked: {
                    if (checked) {
                        title.forceActiveFocus();
                    } else {
                        title.text = "";
                    }
                }
            }

            TextField {
                id: title
                Layout.fillWidth: true
                enabled: showTitle.checked
                placeholderText: i18nc("@info:placeholder", "Enter title")
            }
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
