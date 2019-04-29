    /*
 *  Copyright 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    id: appearancePage

    property alias cfg_showTitle: showTitle.checked
    property alias cfg_title: title.text

    property alias cfg_showSeconds: showSeconds.checked

    property alias cfg_showNotification: showNotification.checked
    property alias cfg_notificationText: notificationText.text

    RowLayout {
        Layout.fillWidth: true

        Kirigami.FormData.label: i18nc("@title:label", "Display:")

        QQC2.CheckBox {
            id: showTitle


            text: i18nc("@option:check", "Show title:");
            onClicked: {
                if (checked) {
                    title.forceActiveFocus();
                }
            }
        }

        QQC2.TextField {
            id: title
            Layout.fillWidth: true
            enabled: showTitle.checked
        }
    }

    QQC2.CheckBox {
        id: showSeconds
        text: i18nc("@option:check", "Show seconds");
    }


    Item {
        Kirigami.FormData.isSection: true
    }


    RowLayout {
        Layout.fillWidth: true

        Kirigami.FormData.label: i18nc("@title:label", "Notifications:")

        QQC2.CheckBox {
            id: showNotification
            text: i18nc("@option:check", "Show notification text:");
            onClicked: {
                if (checked) {
                    notificationText.forceActiveFocus();
                }
            }
        }

        QQC2.TextField {
            id: notificationText
            Layout.fillWidth: true
            enabled: showNotification.checked
        }
    }
}

