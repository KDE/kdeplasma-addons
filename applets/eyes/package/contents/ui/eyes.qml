/***************************************************************************
 *   Copyright (C) 2012 by Davide Bettio <davide.bettio@kdemail.net>       *
 *   Copyright (C) 2008-2009 by Olivier Goffart <ogoffart@kde.org>         *
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

import QtQuick 1.0;
import org.kde.plasma.core 0.1 as PlasmaCore;

Item {
    property int pupilSize: Math.min( Math.min(width / 2, height) / 5, (width / 2 + height) / 12);

    PlasmaCore.Svg {
        id: eyesSvg;
        imagePath: "widgets/eyes";
    }

    PlasmaCore.SvgItem {
        id: leftEye;

        anchors {
            left: parent.left;
            top: parent.top;
            bottom: parent.bottom;
        }
        width: parent.width/2 - 2;

        svg: eyesSvg;
        elementId: "leftEye";

        PlasmaCore.SvgItem {
            id: leftPupil;

            anchors.centerIn: leftEye;

            width: pupilSize;
            height: pupilSize;

            svg: eyesSvg;
            elementId: "leftPupil";
        }
    }

    PlasmaCore.SvgItem {
        id: rightEye;

        anchors {
            right: parent.right;
            top: parent.top;
            bottom: parent.bottom;
        }
        width: parent.width/2 - 2;

        svg: eyesSvg;
        elementId: "rightEye";

        PlasmaCore.SvgItem {
            id: rightPupil;

            anchors.centerIn: rightEye;

            width: pupilSize;
            height: pupilSize;

            svg: eyesSvg;
            elementId: "rightPupil";
        }
    }
}

