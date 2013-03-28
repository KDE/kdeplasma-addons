/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: root
    width: 800
    height: 480
    svg: PlasmaCore.Svg {
        id: wallpaperSvg
        //FIXME: Svg doesn't support relative paths
        imagePath: Qt.resolvedUrl("wallpaper.svgz").substring(7)
    }
    elementId: "background"

    BackgroundElement {
        id: centerLayer0
        elementId: "center-layer0"
    }
    BackgroundElement {
        id: centerLayer1
        elementId: "center-layer1"
    }

    BackgroundElement {
        id: leftLayer0
        elementId: "left-layer0"
    }

    BackgroundElement {
        id: leftLayer1
        elementId: "left-layer1"

        BackgroundElement {
            id: leftLayer2
            elementId: "left-layer2"
        }
        BackgroundElement {
            id: leftLayer3
            elementId: "left-layer3"
        }
        BackgroundElement {
            id: leftLayer4
            elementId: "left-layer4"
        }
    }

    BottomBackgroundElement {
        id: bottomLayer0
        elementId: "bottom-layer0"

        BottomBackgroundElement {
            id: bottomLayer1
            elementId: "bottom-layer1"
        }
        BottomBackgroundElement {
            id: bottomLayer2
            elementId: "bottom-layer2"
        }
    }

    RightBackgroundElement {
        id: rightLayer0
        elementId: "right-layer0"
    }

    RightBackgroundElement {
        id: rightLayer1
        elementId: "right-layer1"

        RightBackgroundElement {
            id: rightLayer2
            elementId: "right-layer2"
        }
        RightBackgroundElement {
            id: rightLayer3
            elementId: "right-layer3"
        }
        RightBackgroundElement {
            id: rightLayer4
            elementId: "right-layer4"
        }
    }

    Timer {
        property int lastLayer: 0
        property variant layers: [centerLayer0, centerLayer1, leftLayer0, leftLayer1, bottomLayer0, rightLayer0, rightLayer1]
        running: true
        repeat: true
        interval: 30000
        onTriggered: {
            layers[lastLayer].change()
            lastLayer = (lastLayer + 1) % 7
        }
    }

}