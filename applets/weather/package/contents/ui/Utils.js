.pragma library

function setAlphaF(solidColor, alpha) {
    var solid = String(solidColor);
    var alphaDecimal = Math.round(alpha * 255);
    return "#" + alphaDecimal.toString(16) + solid.slice(1);
}
