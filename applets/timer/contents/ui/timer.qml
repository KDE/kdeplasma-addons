/***************************************************************************
 *   Copyright 2008,2014 by Davide Bettio <davide.bettio@kdemail.net>      *
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

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as QtExtra

Item
{
    property int seconds : 95;
    property bool running: true;
    property variant predefinedTimers;
    property date startedAt;
    property bool showTitle: false;
    property string title;
    property bool hideSeconds: false;
    property bool showMessage: true;
    property string message;
    property bool runCommand: false;
    property string command;
    property real digits: (hideSeconds) ? 4.5 : 7;
    property int digitH: ((height / 2) * digits < width ? height : ((width - (digits - 1)) / digits) * 2);
    property int digitW: digitH / 2;

    PlasmaCore.Svg {
        id: timerSvg
        imagePath: "widgets/timer"
    }

    Timer {
        id: t;
        interval: 1000;
        onTriggered: seconds--;
        repeat: true;
    }

    Column {
        anchors.centerIn: parent;
        Text {
            id: titleLabel;
            text: title;
            visible: showTitle;
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: parent.parent.height - digitH;
        }
        Row {
            TimerDigit {
                meaning: 60*60*10;
                num: ~~((seconds / (60*60)) / 10);
                suffix: (running && seconds < 60) ? "_1" : "";
            }
            TimerDigit {
                meaning: 60*60;
                num: (seconds / (60*60)) % 10;
                suffix: (running && seconds < 60) ? "_1" : "";
            }

            PlasmaCore.SvgItem {
                svg: timerSvg;
                width: digitW / 2;
                height: digitH;
                elementId: "separator" + ((running && seconds < 60) ? "_1" : "");
            }
            TimerDigit {
                meaning: 600;
                num: ~~(~~((seconds % (60*60)) / 60) / 10);
                suffix: (running && seconds < 60) ? "_1" : "";
            }
            TimerDigit {
                meaning: 60;
                num: ~~((seconds % (60*60)) / 60) % 10;
                suffix: (running && seconds < 60) ? "_1" : "";
            }

            PlasmaCore.SvgItem {
                svg: timerSvg;
                width: digitW / 2;
                height: digitH;
                elementId: "separator" + ((running && seconds < 60) ? "_1" : "");
                visible: !hideSeconds;
            }
            TimerDigit {
                meaning: 10;
                num: ~~((seconds % 60) / 10);
                suffix: (running && seconds < 60) ? "_1" : "";
                visible: !hideSeconds;
            }
            TimerDigit {
                meaning: 1;
                num: (seconds % 60) % 10;
                suffix: (running && seconds < 60) ? "_1" : "";
                visible: !hideSeconds;
            }
        }
    }

    MouseArea {
        anchors.fill: parent;
        onClicked: t.start();
    }

    //PlasmaComponents.ContextMenu {
    //    model: predefinedTimers;
    //}

    //Component.onCompleted: {
    //    plasmoid.addEventListener ('ConfigChanged', configChanged);
    //}

    function configChanged() {
        predefinedTimers = plasmoid.readConfig("predefinedTimers", ["00:00:30", "00:01:00", "00:02:00", "00:05:00", "00:07:30",
                                                                    "00:10:00", "00:15:00", "00:20:00", "00:25:00", "00:30:00",
                                                                    "00:45:00", "01:00:00" ]); 
        startedAt = plasmoid.readConfig("startedAt", new Date());
        showTitle = plasmoid.readConfig("showTitle", false);
        title = plasmoid.readConfig("title", i18n("Timer"));
        hideSeconds = plasmoid.readConfig("hideSeconds", false);
        showMessage = plasmoid.readConfig("showMessage", true);
        message = plasmoid.readConfig("message", i18n("Timer Timeout"));
        runCommand = plasmoid.readConfig("runCommand", false);
        command = plasmoid.readConfig("command", "");
    }
}

