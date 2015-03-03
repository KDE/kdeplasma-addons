/*
 *  Copyright 2015 Marco Martin <mart@kde.org>
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

import QtQuick 2.1
import QtQuick.Controls 1.1 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.plasma.core 2.0 as PlasmaCore


Item {
    id: root
    width: childrenRect.width
    height: childrenRect.height

    signal configurationChanged

    function saveConfig() {
        plasmoid.nativeInterface.arrowsOnHover = showArrowsOnOver.checked;
        plasmoid.nativeInterface.showComicTitle = showComicTitle.checked;
        plasmoid.nativeInterface.showComicIdentifier = showIdentifier.checked;
        plasmoid.nativeInterface.showComicAuthor = showAuthor.checked;
        plasmoid.nativeInterface.showComicUrl = showUrl.checked;

        plasmoid.nativeInterface.saveConfig();
        plasmoid.nativeInterface.configChanged();
    }

    Component.onCompleted: {
        showArrowsOnOver.checked = plasmoid.nativeInterface.arrowsOnHover;
        showComicTitle.checked = plasmoid.nativeInterface.showComicTitle;
        showIdentifier.checked = plasmoid.nativeInterface.showComicIdentifier;
        showAuthor.checked = plasmoid.nativeInterface.showComicAuthor;
        showUrl.checked = plasmoid.nativeInterface.showComicUrl;
    }

    Layouts.ColumnLayout {
        id: mainColumn

        Controls.GroupBox {
            Layouts.Layout.fillWidth: true

            title: i18n("Appearance")
            flat: true

            Layouts.ColumnLayout {
                Controls.CheckBox {
                    id: showArrowsOnOver
                    text: i18n("Show arrows only on mouse over")
                    onCheckedChanged: root.configurationChanged();
                }
            }
        }
        Controls.GroupBox {
            Layouts.Layout.fillWidth: true

            title: i18n("Information")
            flat: true

            Layouts.ColumnLayout {
                Controls.CheckBox {
                    id: showComicTitle
                    text: i18n("Show comic title")
                    onCheckedChanged: root.configurationChanged();
                }
                Controls.CheckBox {
                    id: showIdentifier
                    text: i18n("Show comic identifier")
                    onCheckedChanged: root.configurationChanged();
                }
                Controls.CheckBox {
                    id: showAuthor
                    text: i18n("Show comic author")
                    onCheckedChanged: root.configurationChanged();
                }
                Controls.CheckBox {
                    id: showUrl
                    text: i18n("Show comic URL")
                    onCheckedChanged: root.configurationChanged();
                }
            }
        }
    }
}
