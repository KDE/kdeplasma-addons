/***************************************************************************
 *   Copyright 2014 by Davide Bettio <davide.bettio@kdemail.net>           *
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
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as QtExtra

Item
{
    id: root;

    property string noteText: plasmoid.configuration.noteText;

    Plasmoid.backgroundHints: "NoBackground";

    PlasmaCore.Svg {
        id: notesSvg;
        imagePath: "widgets/notes";
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent;
        svg: notesSvg;
        elementId: "yellow-notes";
    }

    Timer {
        id: saveTimer;
        onTriggered: saveNote();
        interval: 5000;
    }

    TextEdit {
        id: textEdit;

        anchors {
            fill: parent;
            leftMargin: parent.width / 15;
            rightMargin: parent.width / 15;
            topMargin: parent.height / 15;
            bottomMargin: parent.height / 15;
        }

        clip: true;
        text: noteText;
        textFormat: TextEdit.RichText;

        onTextChanged: saveTimer.start();
    }

    function saveNote()
    {
       if (plasmoid.configuration.noteText != textEdit.text){
           plasmoid.configuration.noteText = textEdit.text;
       }
    }
}

