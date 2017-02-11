/***************************************************************************
 *   Copyright 2014, 2016 by Mikhail Ivchenko <ematirov@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtWebKit 3.0
import QtQuick.Layouts 1.1
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    RowLayout{
        Layout.fillWidth: true
        PlasmaComponents.Button{
            iconSource: "go-previous"
            onClicked: webview.goBack()
            enabled: webview.canGoBack
        }
        PlasmaComponents.Button{
            iconSource: "go-next"
            onClicked: webview.goForward()
            enabled: webview.canGoForward
        }
        PlasmaComponents.TextField{
            Layout.fillWidth: true
            onAccepted: {
                var url = text;
                if (url.indexOf(":/") < 0) {
                    url = "http://" + url;
                }
                webview.url = url;
            }
            text: webview.url
        }
        PlasmaComponents.Button{
            iconSource: "view-refresh"
            onClicked: webview.reload()
        }
    }
    PlasmaExtras.ScrollArea {
        Layout.fillWidth: true
        Layout.fillHeight: true
        WebView {
            id: webview
            anchors.fill: parent
            onUrlChanged: plasmoid.configuration.url = url;
            Component.onCompleted: url = plasmoid.configuration.url;
        }
    }
    //There will be RowLayout with buttons for bookmarks and zooming.

}
