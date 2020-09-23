/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        {text: i18nc("@title:menu", "Copy to Clipboard"), section: true},
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
