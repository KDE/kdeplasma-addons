/*
 *  Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
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

var formats = [
    "RRR,GGG,BBB",
    "#rrggbb",
    "rrggbb",
    "#RRGGBB",
    "RRGGBB",
    "rgb",
    "rgba",
    "Qt.rgba",
    "LaTeX"
]

function formatColor(color, format) {
    var hexR = padHex((color.r * 255).toString(16))
    var hexG = padHex((color.g * 255).toString(16))
    var hexB = padHex((color.b * 255).toString(16))

    switch (format) {
    case "RRR,GGG,BBB":
        return [color.r * 255, color.g * 255, color.b * 255].join(", ")
    case "#rrggbb":
        return "#" + formatColor(color, "rrggbb")
    case "rrggbb":
        return formatColor(color, "RRGGBB").toLowerCase()
    case "#RRGGBB":
        return "#" + formatColor(color, "RRGGBB")
    case "RRGGBB":
        return (hexR + hexG + hexB).toUpperCase()
    case "rgb":
        return "rgb(" + formatColor(color, "RRR,GGG,BBB") + ")"
    case "rgba":
        return "rgba(" + formatColor(color, "RRR,GGG,BBB") + ", 1)"
    case "Qt.rgba":
        return "Qt.rgba(" + [roundComponent(color.r), roundComponent(color.g), roundComponent(color.b)].join(", ") + ", 1)"
    case "LaTeX":
        return "\\definecolor{ColorName}{rgb}{" + [roundComponent(color.r), roundComponent(color.g), roundComponent(color.b)].join(",") + "}"
    }
}

function padHex(n) {
    return ("0" + n).substr(-2, 2) // fancy
}

function roundComponent(n) {
    return Math.round(n * 100) / 100
}

function menuForColor(color) {
    return [
        {text: i18n("Copy to Clipboard"), section: true},
        {text: formatColor(color, "RRR,GGG,BBB")},
        {text: formatColor(color, "rgb")},
        {text: formatColor(color, "rgba")},
        {separator: true},
        {text: formatColor(color, "#rrggbb")},
        {text: formatColor(color, "rrggbb")},
        {text: formatColor(color, "#RRGGBB")},
        {text: formatColor(color, "RRGGBB")},
        {separator: true},
        {text: formatColor(color, "Qt.rgba")},
        {text: formatColor(color, "LaTeX")}
    ]
}
